/*
 * Test program for GDSS.
 *
 */

#include <krb.h>
#include <gdss.h>

char TestMessage[] = "This is a test message. This is only a test.";

unsigned char OldSig[] = { 0x44, 0x6a, 0x69, 0x73, 0x20, 0x21, 0x20,
0x21, 0x41, 0x54, 0x48, 0x45, 0x4e, 0x41, 0x2e, 0x4d, 0x49, 0x54,
0x2e, 0x45, 0x44, 0x55, 0x20, 0x21, 0x2a, 0x13, 0x28, 0x97, 0x2b,
0x47, 0xad, 0xcc, 0xb0, 0xe1, 0x11, 0x71, 0xc1, 0x48, 0xa5, 0xd1,
0x1e, 0xa6, 0x20, 0x20, 0x97, 0x3d, 0xcb, 0x73, 0xf3, 0x63, 0x52,
0x3b, 0x2c, 0x7f, 0x05, 0x30, 0xe0, 0x03, 0x75, 0x9e, 0x0c, 0xa7,
0xdc, 0xf2, 0x41, 0x89, 0x95, 0xb1, 0x4e, 0xe1, 0x11, 0xba, 0x05,
0x39, 0xd1, 0xf1, 0x74, 0x96, 0x7f, 0xa2, 0xaf, 0x40, 0xc3, 0x70,
0x5a, 0x39, 0xa0, 0xce, 0x8a, 0xfd, 0x1c, 0xed, 0xcc, 0x00 };

main(argc, argv)
int argc;
char **argv;
{
  char Signature[1000];
  int status;
  SigInfo aSigInfo;

  printf("Performing Test Signature....");
  status = GDSS_Sign(TestMessage, strlen(TestMessage), Signature);
  if (status != GDSS_SUCCESS) {
    printf("FAILED\n");
    if (status == GDSS_E_TIMEDOUT)
      printf("Signature Server Request timed out (is it running?)!\n");
    else printf("GDSS_Sign Error number: %d\n", status);
    exit (1);
  } else {
    printf("SUCCESS\n");
  }

  memset(&aSigInfo, 0, sizeof(aSigInfo));

  printf("Attempting to Verify signature...");
  status = GDSS_Verify(TestMessage, strlen(TestMessage), Signature,
		       &aSigInfo);
  if (status != GDSS_SUCCESS) {
    printf("FAILED\n");
    printf("GDSS_Verify Error number: %d\n", status);
    exit (1);
  } else {
    printf("SUCCESS\n");
    printf("Signature by: %s.%s@%s at %s\n",
	   aSigInfo.pname, aSigInfo.pinst, aSigInfo.prealm,
	   ctime(&aSigInfo.timestamp));
  }

  printf("Purposely damaging data...test: ");

  TestMessage[0] = 'S';

  status = GDSS_Verify(TestMessage, strlen(TestMessage), Signature,
		       &aSigInfo);
  if (status == GDSS_SUCCESS) {
    printf("FAILED\n");
    printf("Signature succeeded when it should *not* have!\n");
    exit (1);
  } else if (status != GDSS_E_BADSIG) {
    printf("FAILED\n");
    printf("Signature failed, but returned error code %d\n", status);
    exit (1);
  } else {
    printf("PASSED\n");
  }

  printf("Fixing data...\n");

  TestMessage[0] = 'T';

  printf("Attempting to Verify signature...");
  status = GDSS_Verify(TestMessage, strlen(TestMessage), Signature,
		       &aSigInfo);
  if (status != GDSS_SUCCESS) {
    printf("FAILED\n");
    printf("GDSS_Verify Error number: %d\n", status);
    exit (1);
  } else {
    printf("SUCCESS\n");
    printf("Signature by: %s.%s@%s at %s\n",
	   aSigInfo.pname, aSigInfo.pinst, aSigInfo.prealm,
	   ctime(&aSigInfo.timestamp));
  }

  printf("Testing Builtin Signature...\n");

  printf("Attempting to Verify signature...");

  status = GDSS_Verify(TestMessage, strlen(TestMessage), OldSig,
		       &aSigInfo);
  if (status != GDSS_SUCCESS) {
    printf("FAILED\n");
    printf("GDSS_Verify Error number: %d\n", status);
    exit (1);
  } else {
    printf("SUCCESS\n");
    printf("Signature by: %s.%s@%s at %s\n",
	   aSigInfo.pname, aSigInfo.pinst, aSigInfo.prealm,
	   ctime(&aSigInfo.timestamp));
  }

  printf("All Test Passed.\n");
  exit (0);
}






