/*
 * Copyright (C) 1998-2000 Luke Howard. All rights reserved.
 * CONFIDENTIAL
 * $Id: ldap_adgssapi_bind.c,v 1.4 2003-05-23 16:38:16 zacheiss Exp $
 *
 * Implementation of GSS-API client side binding for SASL
 */

#ifdef _WIN32
#include <windows.h>
#endif
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#ifndef _WIN32
#include <sys/uio.h>
#include <sys/socket.h>
#include <netinet/in.h>
#endif

#include <arpa/nameser.h>
#include <resolv.h>
#include <lber.h>
#include <krb5.h>
#include "ldap-int.h"
#include "gssldap-int.h"

#ifndef _WIN32
typedef gss_uint32  OM_uint32;
#endif

char ldap_domain_name[128];

#ifndef T_SRV
#define T_SRV 33
#endif
#define LDAP_SERVICE	"_ldap"
#define TCP_PROTOCOL	"_tcp"

int locate_ldap_server(char *domain, char **server_name);
int ldap_delete_tickets(LDAP *ld, char *service_name);
static int negotiate_security_options(gssldap_client_state_t state, 
                                      int layer);
int ldap_reset_principal(LDAP *ld, char *service_name, gss_name_t target_name);
unsigned long gsssasl_pack_security_token(OM_uint32 *min_stat, gss_ctx_id_t context,
                                          gsssasl_security_negotiation_t inmask,
                                          size_t masklength, gss_buffer_t wrapped_tok);

int gsssasl_unpack_security_token(OM_uint32 *min_stat, gss_ctx_id_t context,
                                  gss_buffer_t wrapped_tok,
                                  gsssasl_security_negotiation_t *outmask,
                                  size_t *masklength);

#ifdef GSSSASL_DEBUG
static int debug_ = 1;
#define TRACE(x) do { if (debug_) fprintf(stderr, "%s\n", x); fflush(stderr); } while (0);
#define LDAP_PERROR(ld, x) do { if (debug_) ldap_perror(ld, x); } while (0);
#define LOG_STATUS(msg, min, maj) log_status(msg, min, maj)
#else
#define TRACE(x)
#define LDAP_PERROR(ld, x)
#define LOG_STATUS(msg, min, maj)
#endif  /* GSSSASL_DEBUG */

#ifdef HACK_SERVICE_NAME
char *__service = NULL;
#endif

/*
 * The read and write fns need to access the context in here and
 * there doesn't appear to be a way to pass this info to them.
 */
static gss_ctx_id_t security_context;
static int security_layer = 0;
static unsigned long security_token_size = 0;

LDAP_CALLBACK int LDAP_C ldap_gssapi_read(LBER_SOCKET sock, void *data,
                                          int len)
{
  OM_uint32   maj_stat;
  OM_uint32   min_stat;
  OM_uint32   rc;
  OM_uint32   pdulen;
  gss_buffer_desc recv_tok;
  gss_buffer_desc wrapped_tok;
  int         count;

  if (security_layer & 
    (GSSSASL_INTEGRITY_PROTECTION | GSSSASL_PRIVACY_PROTECTION))
    {
      if (recv(sock, (char *)&pdulen, sizeof(pdulen), 0) != sizeof(pdulen))
        return (0);
      wrapped_tok.length = ntohl(pdulen);
#ifdef GSSSASL_DEBUG
      if (debug_)
        {
          fprintf(stderr, "Reading data of %d octets\n",
                  wrapped_tok.length);
        }
#endif  /* GSSSASL_DEBUG */
      if ((int)wrapped_tok.length > security_token_size)
        return (0);
      wrapped_tok.value = malloc(wrapped_tok.length);
      if (wrapped_tok.value == NULL)
        return (0);
      count = recv(sock, wrapped_tok.value, wrapped_tok.length, 0);
      if (count != (int)wrapped_tok.length)
        {
          gss_release_buffer(&rc, &wrapped_tok);
          return (0);
        }
      maj_stat = gss_unwrap(&min_stat,
                            security_context,
                            &wrapped_tok,
                            &recv_tok,
                            &rc,
                            (gss_qop_t *) NULL);
      if (maj_stat != GSS_S_COMPLETE)
        {
          gss_release_buffer(&rc, &wrapped_tok);
          return maj_stat;
        }
#ifdef GSSSASL_DEBUG
      if (debug_)
        {
          fprintf(stderr, "Got %d bytes of %s data\n",
                  recv_tok.length, rc ? "private" : "signed");
        }
#endif  /* GSSSASL_DEBUG */
      if ((int)recv_tok.length > len)
        {
          gss_release_buffer(&rc, &recv_tok);
          gss_release_buffer(&rc, &wrapped_tok);
          return GSS_S_FAILURE;
        }
      memcpy(data, recv_tok.value, recv_tok.length);
      pdulen = recv_tok.length;
      gss_release_buffer(&rc, &recv_tok);
      gss_release_buffer(&rc, &wrapped_tok);

      return pdulen;
    } 
  else
    return read(sock, data, len);
}

LDAP_CALLBACK int LDAP_C ldap_gssapi_write(LBER_SOCKET sock, const void  *data,
                                           int len)
{
  OM_uint32   maj_stat;
  OM_uint32   min_stat;
  OM_uint32   pdulen;
  OM_uint32   rc;
  gss_buffer_desc send_tok;
  gss_buffer_desc wrapped_tok;
  unsigned char   *ptr;

  if (security_layer & 
      (GSSSASL_INTEGRITY_PROTECTION | GSSSASL_PRIVACY_PROTECTION))
    {
      send_tok.length = len;
      send_tok.value = (void *) data;
#ifdef GSSSASL_DEBUG
      if (debug_)
        {
          fprintf(stderr, "Sending %d bytes of data\n",
                  len);
        }
#endif  /* GSSSASL_DEBUG */
      maj_stat = gss_wrap(&min_stat,
                          security_context,
                          (security_layer & GSSSASL_PRIVACY_PROTECTION) ? 1 : 0,
                          GSS_C_QOP_DEFAULT,
                          &send_tok,
                          &rc,
                          &wrapped_tok);
      if (maj_stat != GSS_S_COMPLETE)
        return maj_stat;
#ifdef GSSSASL_DEBUG
      if (debug_)
        {
          fprintf(stderr, "Sent %d bytes of %s data\n",
                  wrapped_tok.length, rc ? "private" : "signed");
        }
#endif  /* GSSSASL_DEBUG */
      pdulen = htonl(wrapped_tok.length);
      ptr = calloc(1, sizeof(pdulen) + wrapped_tok.length);
      memcpy(ptr, &pdulen, sizeof(pdulen));
      memcpy(&ptr[sizeof(pdulen)], wrapped_tok.value, wrapped_tok.length);
      rc = send(sock, ptr, sizeof(pdulen) + wrapped_tok.length, 0);
      free(ptr);
      if (rc == (wrapped_tok.length + sizeof(pdulen)))
        rc = len;
      gss_release_buffer(&maj_stat, &wrapped_tok);
      return rc;
    } 
  else
    return send(sock, data, len, 0);
}

#ifdef GSSSASL_DEBUG
/*
 * Dump the token to stderr
 */
static void print_token(gss_buffer_t tok)
{
  int i;
  unsigned char *p = tok->value;

  for (i = 0; i < (int)tok->length; i++, p++)
    {
      fprintf(stderr, "%02x ", *p);
      if ((i % 16) == 15)
        {
          fprintf(stderr, "\n");
        }
    }
  fprintf(stderr, "\n");
  fflush(stderr);
  return;
}

/*
 * Handle a GSS-API error
 */
static void log_status_impl(const char *reason, OM_uint32 res, int type)
{
  OM_uint32 maj_stat, min_stat;
  gss_buffer_desc msg;
  OM_uint32 msg_ctx;

  msg_ctx = 0;
  while (1)
    {
      maj_stat = gss_display_status(&min_stat,
                                    res,
                                    type,
                                    GSS_C_NULL_OID,
                                    &msg_ctx,
                                    &msg);

      if (debug_)
        {
          fprintf(stderr, "ldap_adgssapi_bind: %s: %s\n",
                  (char *) msg.value,
                  reason);
        }
      (void) gss_release_buffer(&min_stat, &msg);
      if (!msg_ctx)
        break;
    }
  return;
}

/*
 * Cover function to handle a GSS-API error
 */
static void log_status(const char *reason, OM_uint32 maj_stat,
                       OM_uint32 min_stat)
{
  log_status_impl(reason, maj_stat, GSS_C_GSS_CODE);
  log_status_impl(reason, min_stat, GSS_C_MECH_CODE);
  return;
}
#endif  /* GSSSASL_DEBUG */

/*
 * Send a GSS-API token as part of a SASL BindRequest
 */
static int send_token(gssldap_client_state_t state, gss_buffer_t send_tok)
{
  struct berval cred;

  TRACE("==> send_token");

  cred.bv_val = send_tok->value;
  cred.bv_len = send_tok->length;

  if (ldap_sasl_bind(state->ld,
                     state->binddn,
                     GSSAPI_SASL_NAME,
                     &cred,
                     NULL,
                     NULL,
                     &state->msgid) != LDAP_SUCCESS)
        {
      LDAP_PERROR(state->ld, "send_token");
      TRACE("<== send_token");
      return -1;
    }
  TRACE("<== send_token");
  return 0;
}

/*
 * Parse the final result sent back from the server.
 */
static int parse_bind_result(gssldap_client_state_t state)
{
  LDAPMessage *res = NULL, *msg = NULL;
  int rc;

  TRACE("==> parse_bind_result");

  if (ldap_result(state->ld,
                  state->msgid,
                  LDAP_MSG_ALL,
                  NULL,
                  &res) <= 0) 
    {
      LDAP_PERROR(state->ld, "ldap_result");
      TRACE("<== parse_bind_result");
      return -1;
    }
  for (msg = ldap_first_message(state->ld, res);
  msg != NULL;
  msg = ldap_next_message(state->ld, msg)) 
    {
      if (ldap_msgtype(msg) == LDAP_RES_BIND)
        {
          ldap_parse_result(state->ld, msg, &rc, NULL, NULL, NULL, NULL, 0);
          break;
        }
     }

  ldap_msgfree(res);
  state->msgid = -1;

  TRACE("<== parse_bind_result");

  if (rc == LDAP_SUCCESS) 
    {
      return 0;
    }
  state->rc = rc;
  return -1;
}

/*
 * Receive a GSS-API token from a SASL BindResponse
 * The contents of recv_tok must be freed by the 
 * caller.
 */
static int recv_token(gssldap_client_state_t state, gss_buffer_t recv_tok)
{
  struct berval *servercred = NULL;
  LDAPMessage *res = NULL, *msg = NULL;
  int rc;

  TRACE("==> recv_token");

  if (ldap_result(state->ld, state->msgid, LDAP_MSG_ALL, NULL, &res) <= 0) 
    {
      LDAP_PERROR(state->ld, "ldap_result");
      TRACE("<== recv_token");
      return -1;
    }
  recv_tok->value = NULL;
  recv_tok->length = 0;

  for (msg = ldap_first_message(state->ld, res);
  msg != NULL;
  msg = ldap_next_message(state->ld, msg)) 
    {
    if (ldap_msgtype(msg) == LDAP_RES_BIND && servercred == NULL) 
      {
        rc = ldap_parse_sasl_bind_result(state->ld,
                                         msg,
                                         &servercred,
                                         0);
        if (rc == LDAP_SUCCESS && servercred != NULL) 
          {
            recv_tok->value = malloc(servercred->bv_len);
            if (recv_tok->value != NULL) 
              {
                memcpy(recv_tok->value, servercred->bv_val, servercred->bv_len);
                recv_tok->length = servercred->bv_len;
              }
            break;
          } 
        else 
          {
            state->rc = rc;
          }
      }
  }

  ldap_msgfree(res);
  state->msgid = -1;
  TRACE("<== recv_token");
  if (servercred != NULL) 
    {
       nslberi_free(servercred);
      TRACE("<== recv_token");
      return 0;
      }
  if (state->rc == LDAP_SUCCESS) 
    {
      state->rc = LDAP_OPERATIONS_ERROR;
    } 
  else 
    {
      LDAP_PERROR(state->ld, "recv_token");
    }

  TRACE("<== recv_token");
  return -1;
}

/*
 * The client calls GSS_Init_sec_context, passing in 0 for
 * input_context_handle (initially) and a targ_name equal to output_name
 * from GSS_Import_Name called with input_name_type of
 * GSS_C_NT_HOSTBASED_SERVICE and input_name_string of
 * "service@hostname" where "service" is the service name specified in
 * the protocol's profile, and "hostname" is the fully qualified host
 * name of the server.  The client then responds with the resulting
 * output_token.  If GSS_Init_sec_context returns GSS_S_CONTINUE_NEEDED,
 * then the client should expect the server to issue a token in a
 * subsequent challenge.  The client must pass the token to another call
 * to GSS_Init_sec_context, repeating the actions in this paragraph.
 */
static int client_establish_context(LDAP *ld, gssldap_client_state_t state, 
                                    char *service_name)
{
  gss_buffer_desc send_tok;
  gss_buffer_desc recv_tok;
  gss_buffer_desc *token_ptr;
  gss_name_t      target_name;
  OM_uint32       maj_stat;
  OM_uint32       min_stat;
  gss_OID         oid = GSS_C_NULL_OID;
  OM_uint32       ret_flags;

  TRACE("==> client_establish_context");

  memset(&recv_tok, '\0', sizeof(recv_tok));
  send_tok.value = service_name;
  send_tok.length = strlen(send_tok.value) + 1;

  maj_stat = gss_import_name(&min_stat,
                             &send_tok,
                             (gss_OID) gss_nt_service_name,
                             &target_name);
  if (ldap_reset_principal(ld, service_name, target_name))
    {
      TRACE("<== client_establish_context");
      return -1;
    }

  token_ptr = GSS_C_NO_BUFFER;
  state->context = GSS_C_NO_CONTEXT;

  do 
    {
      maj_stat = gss_init_sec_context(&min_stat,
                                        GSS_C_NO_CREDENTIAL,
                                        &state->context,
                                        target_name,
                                        oid,
                                        GSS_C_MUTUAL_FLAG | GSS_C_REPLAY_FLAG,
                                        0,
                                        NULL,
                                        token_ptr,
                                        NULL,
                                        &send_tok,
                                        &ret_flags,
                                        NULL);

      if (token_ptr != GSS_C_NO_BUFFER)
        (void) gss_release_buffer(&min_stat, &recv_tok);

      if (maj_stat != GSS_S_COMPLETE && maj_stat != GSS_S_CONTINUE_NEEDED)
        {
          LOG_STATUS("initializing context", maj_stat, min_stat);
          (void) gss_release_name(&min_stat, &target_name);
          TRACE("<== client_establish_context");
          return -1;
        }
#ifdef GSSSASL_DEBUG
      if (debug_) 
        {
          fprintf(stderr, "Sending init_sec_context token (size=%d)...\n",
          send_tok.length);
          fflush(stderr);
          print_token(&send_tok);
        }
#endif
      if (send_token(state, &send_tok) < 0)
        {
          TRACE("Send_token failed");
          (void) gss_release_buffer(&min_stat, &send_tok);
          (void) gss_release_name(&min_stat, &target_name);
          TRACE("<== client_establish_context");
          return -1;
        }
      (void) gss_release_buffer(&min_stat, &send_tok);
      if (maj_stat == GSS_S_CONTINUE_NEEDED) 
        {
          TRACE("continue needed...");
          if (recv_token(state, &recv_tok) < 0) 
            {
              TRACE("recv_token failed");
              (void) gss_release_name(&min_stat, &target_name);
              TRACE("<== client_establish_context");
              return -1;
            }
#ifdef GSSSASL_DEBUG
          if (debug_)
            {
              fprintf(stderr, "Received token (size=%d)...\n",
              recv_tok.length);
              fflush(stderr);
              print_token(&recv_tok);
            }
#endif
          token_ptr = &recv_tok;
        }
    } while (maj_stat == GSS_S_CONTINUE_NEEDED);

  (void) gss_release_name(&min_stat, &target_name);

  TRACE("<== client_establish_context");
  return 0;
}

/*
 * When GSS_Init_sec_context returns GSS_S_COMPLETE, the client takes
 * the following actions: If the last call to GSS_Init_sec_context
 * returned an output_token, then the client responds with the
 * output_token, otherwise the client responds with no data.  The client
 * should then expect the server to issue a token in a subsequent
 * challenge.  The client passes this token to GSS_Unwrap and interprets
 * the first octet of resulting cleartext as a bit-mask specifying the
 * security layers supported by the server and the second through fourth
 * octets as the maximum size output_message to send to the server.  The
 * client then constructs data, with the first octet containing the
 * bit-mask specifying the selected security layer, the second through
 * fourth octets containing in network byte order the maximum size
 * output_message the client is able to receive, and the remaining
 * octets containing the authorization identity.  The client passes the
 * data to GSS_Wrap with conf_flag set to FALSE, and responds with the
 * generated output_message.  The client can then consider the server
 * authenticated.
 */
static int negotiate_security_options(gssldap_client_state_t state, 
                                      int layer)
{
  OM_uint32       maj_stat;
  OM_uint32       min_stat;
  gss_buffer_desc recv_tok;
  gss_buffer_desc send_tok;
  OM_uint32       rc;
  OM_uint32       ret;
  size_t          mask_length;
  gsssasl_security_negotiation_t  send_mask;
  gsssasl_security_negotiation_t  recv_mask;

  TRACE("==> negotiate_security_options");

  memset(&send_tok, '\0', sizeof(send_tok));
  memset(&recv_tok, '\0', sizeof(recv_tok));
  if ((rc = recv_token(state, &recv_tok)) < 0)
    {
      TRACE("<== negotiate_security_options (recv_token failed)");
      return rc;
    }
#ifdef GSSSASL_DEBUG
  if (debug_)
    {
      fprintf(stderr, "Received token (size=%d)...\n",
      recv_tok.length);
      fflush(stderr);
      print_token(&recv_tok);
      }
#endif  /* GSSSASL_DEBUG */

  maj_stat = gsssasl_unpack_security_token(&min_stat,
                                           state->context,
                                           &recv_tok,
                                           &recv_mask,
                                           &mask_length);

  if (maj_stat != GSS_S_COMPLETE)
    {
      LOG_STATUS("unpacking security negotiation token",
                 maj_stat,
                 min_stat);
      TRACE("<== negotiate_security_options (unpack failed)");
      return -1;
    }
#ifdef GSSSASL_DEBUG
  if (debug_) 
    {
      fprintf(stderr, "Received security token level %d size %d\n",
      recv_mask->security_layer,
      recv_mask->token_size);
      }
#endif
  if ((~recv_mask->security_layer) & layer) 
    {
      free(recv_mask);
      TRACE("<== negotiate_security_options (unsupported security layer)");
      return -1;
      }
  mask_length = sizeof(recv_mask);
  send_mask = NSLDAPI_MALLOC(mask_length);
  if (send_mask == NULL) 
    {
      free(recv_mask);
      TRACE("<== negotiate_security_options (malloc failed)");
      return -1;
    }
  memset(send_mask, '\0', mask_length);
  send_mask->security_layer = layer;
  send_mask->token_size = recv_mask->token_size;
  memcpy(send_mask->identity, "", strlen(""));
  free(recv_mask);

#ifdef GSSSASL_DEBUG
  if (debug_) 
    {
      fprintf(stderr, "Sending security token level %d size %d\n",
      send_mask->security_layer,
      send_mask->token_size);
    }
#endif
  maj_stat = gsssasl_pack_security_token(&min_stat,
                                         state->context,
                                         send_mask,
                                         mask_length,
                                         &send_tok);
  if (maj_stat != GSS_S_COMPLETE)
    {
      LOG_STATUS("packing security negotiation token", maj_stat, min_stat);
      NSLDAPI_FREE(send_mask);
      TRACE("<== negotiate_security_options (pack failed)");
      return -1;
    }
  if ((rc = send_token(state, &send_tok)) < 0)
    {
      NSLDAPI_FREE(send_mask);
      TRACE("<== negotiate_security_options (send_token failed)");
      return rc;
    }
  rc = parse_bind_result(state);

  ret = rc;
  if (rc == 0) 
    {
      security_context = state->context;
      security_layer = layer;
      security_token_size = ntohl(send_mask->token_size);
#ifdef _WIN32
      security_token_size >>= 8;
#endif
      }

  NSLDAPI_FREE(send_mask);
  if (send_tok.value != NULL)
    gss_release_buffer(&rc, &send_tok);
  if (recv_tok.value != NULL)
    gss_release_buffer(&rc, &recv_tok);
  TRACE("<== negotiate_security_options");
  return ret;
}

#ifdef GSSSASL_DEBUG
/*
 * Temporary function to enable debugging
 */
LDAP_API(int)
LDAP_CALL ldap_gssapi_debug(int on)
{
  int old = debug_;
  debug_ = on;
  return old;
}
#endif  /* GSSSASL_DEBUG */

/*
 * Public function for doing a GSS-API SASL bind
 */
LDAP_API(int)
LDAP_CALL ldap_adgssapi_bind(LDAP *ld, const char *who, int layer)
{
  gssldap_client_state_desc state;
  char        *service_name;
  OM_uint32   min_stat;
  int         rc;
  int         i;
  struct ldap_io_fns iofns;
  char        *realm;

  if (!NSLDAPI_VALID_LDAP_POINTER(ld) || ld->ld_defhost == NULL) 
    {
      return -1;
    }

  for (i = 0; i < (int)strlen(ld->ld_defhost); i++)
    ld->ld_defhost[i] = toupper(ld->ld_defhost[i]);

  service_name = 
    NSLDAPI_MALLOC(sizeof(GSSAPI_LDAP_SERVICE_NAME "@") + strlen(ld->ld_defhost));
  if (service_name == NULL) 
    {
      return -1;
    }
  strcpy(service_name, GSSAPI_LDAP_SERVICE_NAME "@");
  realm = strdup(ldap_domain_name);
  for (i = 0; i < (int)strlen(realm); i++)
      realm[i] = toupper(realm[i]);
  strcat(service_name, realm);
  free(realm);

#ifdef GSSSASL_DEBUG
  if (debug_) 
    {
      fprintf(stderr, "LDAP service name: %s\n", service_name);
      fflush(stderr);
    }
#endif

  state.msgid = -1;
  state.ld = ld;
  state.binddn = who;
  state.context = GSS_C_NO_CONTEXT;
  state.rc = LDAP_OPERATIONS_ERROR;

  rc = client_establish_context(ld, &state, service_name);
  if (rc == 0) 
    {
      rc = negotiate_security_options(&state, layer);
    }
  if (rc == 0) 
    {
#ifdef GSSSASL_DEBUG
      gss_buffer_desc tname;
      gss_buffer_desc sname;
      gss_name_t      targ_name;
      gss_name_t      src_name;
      OM_uint32       context_flags;
      OM_uint32       lifetime;
      OM_uint32       maj_stat;
      int             is_open;
      int             is_local;
      gss_OID         mechanism;
      gss_OID         name_type;

      maj_stat = gss_inquire_context(&min_stat,
                                      state.context,
                                      &src_name,
                                      &targ_name,
                                      &lifetime,
                                      &mechanism,
                                      &context_flags,
                                      &is_local,
                                      &is_open);
      if (maj_stat != GSS_S_COMPLETE) 
        {
          LOG_STATUS("inquiring context", maj_stat, min_stat);
          return LDAP_OPERATIONS_ERROR;
        }
      maj_stat = gss_display_name(&min_stat,
                                  src_name,
                                  &sname,
                                  &name_type);
      if (maj_stat != GSS_S_COMPLETE) 
        {
          LOG_STATUS("displaying source name", maj_stat, min_stat);
          return LDAP_OPERATIONS_ERROR;
        }
      maj_stat = gss_display_name(&min_stat,
                                  targ_name,
                                  &tname,
                                  (gss_OID *) NULL);
      if (maj_stat != GSS_S_COMPLETE) 
        {
          LOG_STATUS("displaying target name", maj_stat, min_stat);
          return LDAP_OPERATIONS_ERROR;
        }
      if (debug_) 
        {
          fprintf(stderr, 
                  "\"%.*s\" to \"%.*s\", lifetime %d, flags %x, %s, %s\n",
                  (int) sname.length, (char *) sname.value,
                  (int) tname.length, (char *) tname.value, lifetime,
                  context_flags,
                  (is_local) ? "locally initiated" : "remotely initiated",
                  (is_open) ? "open" : "closed");
          fflush(stderr);
        }
      (void) gss_release_name(&min_stat, &src_name);
      (void) gss_release_name(&min_stat, &targ_name);
      (void) gss_release_buffer(&min_stat, &sname);
      (void) gss_release_buffer(&min_stat, &tname);
#endif
      state.rc = LDAP_SUCCESS;
    }

  if (state.rc == LDAP_SUCCESS)
    {
      if (layer == GSSSASL_PRIVACY_PROTECTION)
        {
          memset(&iofns, 0, sizeof(iofns));
          iofns.liof_read = ldap_gssapi_read;
          iofns.liof_write = ldap_gssapi_write;
          state.rc = ldap_set_option(ld, LDAP_OPT_IO_FN_PTRS, &iofns);
        }
    }

  NSLDAPI_FREE(service_name);
  LDAP_SET_LDERRNO(ld, state.rc, NULL, NULL);

  return state.rc;
}

/* Wrap and encode a security negotiation token */
unsigned long gsssasl_pack_security_token(OM_uint32 *min_stat, 
                             gss_ctx_id_t context,
                             gsssasl_security_negotiation_t inmask,
                             size_t masklength, gss_buffer_t wrapped_tok)
{
  OM_uint32       maj_stat;
  OM_uint32       rc;
  gss_buffer_desc send_tok;

  wrapped_tok->length = 0;
  wrapped_tok->value = NULL;

/*
  if (masklength < sizeof(gsssasl_security_negotiation_desc))
    return GSS_S_FAILURE;
*/

  inmask->token_size = inmask->token_size;

  send_tok.length = masklength;
  send_tok.value = inmask;

  maj_stat = gss_wrap(min_stat,
                      context,
                      0,
                      GSS_C_QOP_DEFAULT,
                      &send_tok,
                      (int *)&rc,
                      wrapped_tok);

/*  inmask->token_size = ntohl(inmask->token_size);*/

  return maj_stat;
}

/* Unwrap and decode a security negotiation token. */
int gsssasl_unpack_security_token(OM_uint32 *min_stat, gss_ctx_id_t context, 
                                  gss_buffer_t wrapped_tok,
                                  gsssasl_security_negotiation_t *outmask, 
                                  size_t *masklength)
{
  OM_uint32       maj_stat;
  OM_uint32       rc;
  gss_buffer_desc recv_tok;

  *masklength = 0;
  *outmask = NULL;
  memset(&recv_tok, '\0', sizeof(recv_tok));

  maj_stat = gss_unwrap(min_stat,
                        context,
                        wrapped_tok,
                        &recv_tok,
                        (int *)&rc,
                        (gss_qop_t *) NULL);
  if (maj_stat != GSS_S_COMPLETE)
    return maj_stat;

/*
#ifdef _WIN32
  if (recv_tok.length < sizeof(gsssasl_security_negotiation_desc)) 
    {
      gss_release_buffer(&rc, &recv_tok);
      return GSS_S_FAILURE;
    }
#endif
*/

  /*
   * we're lazy and don't copy the token. This could cause
   * problems if libgssapi uses a different malloc!
   */
  *masklength = recv_tok.length;
  *outmask = (gsssasl_security_negotiation_t) recv_tok.value;
  if (*outmask == NULL) 
    {
      gss_release_buffer(&rc, &recv_tok);
      return GSS_S_FAILURE;
    }
  return GSS_S_COMPLETE;
}

int ldap_reset_principal(LDAP *ld, char *service_name, gss_name_t target_name)
{
  krb5_context    context = NULL;
  krb5_principal  princ;
  krb5_principal  princ1;
  krb5_principal  temp_princ;
  char            *realm;
  char            *server_name = NULL;
  int             i;

  princ = (krb5_principal)(target_name);

  if (krb5_init_context(&context))
    return(-1);

  realm = strdup(ldap_domain_name);
  for (i = 0; i < (int)strlen(realm); i++)
    realm[i] = toupper(realm[i]);
  server_name = strdup(ld->ld_defhost);
  for (i = 0; i < (int)strlen(server_name); i++)
    server_name[i] = tolower(server_name[i]);

  temp_princ = malloc(sizeof(*princ));
  memcpy(temp_princ, princ, sizeof(*princ));

  krb5_build_principal(context, &princ1, strlen(realm), realm, "ldap", 
                       server_name, ldap_domain_name, 0);

  memcpy(princ, princ1, sizeof(*princ1));
  free(princ1);
  krb5_free_principal(context, temp_princ);

  if (realm != NULL)
    free(realm);
  if (server_name != NULL)
    free(server_name);
  if (context != NULL)
    krb5_free_context(context);
  return(0);
}

int ldap_delete_tickets(LDAP *ld, char *service_name)
{
  int             i;
  int             rc;
  krb5_context    context = NULL;
  krb5_ccache     v5Cache = NULL;
  krb5_creds      creds;
  krb5_cc_cursor  v5Cursor;
  krb5_error_code code;
  char            *sServerName;

  if (!NSLDAPI_VALID_LDAP_POINTER(ld) || ld->ld_defhost == NULL) 
    {
      return -1;
    }

  for (i = 0; i < (int)strlen(ld->ld_defhost); i++)
    ld->ld_defhost[i] = toupper(ld->ld_defhost[i]);

  rc = -1;

  if (krb5_init_context(&context))
    return(rc);

  if (krb5_cc_default(context, &v5Cache))
    goto cleanup;
  if (krb5_cc_start_seq_get(context, v5Cache, &v5Cursor))
    goto cleanup;

  memset(&creds, '\0', sizeof(creds));

  while (!(code = krb5_cc_next_cred(context, v5Cache, &v5Cursor, &creds))) 
    {
      if (krb5_unparse_name(context, creds.server, &sServerName))
        {
          krb5_free_cred_contents(context, &creds);
          continue;
        }
      if (!memcmp(sServerName, service_name, strlen(service_name)))
        {
          krb5_cc_remove_cred(context, v5Cache, 0, &creds);
        }
      continue;
    }

  if ((code == KRB5_CC_END) || (code == KRB5_CC_NOTFOUND))
    {
      krb5_cc_end_seq_get(context, v5Cache, &v5Cursor);
    }
  rc = 0;

cleanup:
  if ((v5Cache != NULL) && (context != NULL))
    krb5_cc_close(context, v5Cache);
  if (context != NULL)
    krb5_free_context(context);
  return(rc);
}

int locate_ldap_server(char *domain, char **server_name)
{
  char  service[128];
  char  host[128];
  int   location_type;
  int   length;
  int   rc;
  int   return_code;
  int   entry_length;
  int   server_count;
  unsigned char   reply[1024];
  unsigned char   *ptr;
    
  strcpy(ldap_domain_name, domain);
  sprintf(service, "%s.%s.%s.", LDAP_SERVICE, TCP_PROTOCOL, domain);

  return_code = -1;
  server_count = 0;
  memset(reply, '\0', sizeof(reply));
  length = res_search(service, C_IN, T_SRV, reply, sizeof(reply));
  if (length >= 0)
    {
      ptr = reply;
      ptr += sizeof(HEADER);
      if ((rc = dn_expand(reply, reply + length, ptr, host, 
                          sizeof(host))) < 0)
        return(-1);
      ptr += (rc + 4);

      while (ptr < reply + length)
        {
          if ((rc = dn_expand(reply, reply + length, ptr, host, 
                              sizeof(host))) < 0)
            break;
          ptr += rc;
          location_type = (ptr[0] << 8) | ptr[1];
          ptr += 8;
	        entry_length = (ptr[0] << 8) | ptr[1];
          ptr += 2;
          if (location_type == T_SRV)
            {
              if ((rc = dn_expand(reply, reply + length, ptr + 6, host, 
                                  sizeof(host))) < 0)
                return -1;
              
              (*server_name) = strdup(host);
              ++server_name;
              return_code = 1;
              server_count++;
            }
        ptr += entry_length;
        }
    }
  return(return_code);
}
