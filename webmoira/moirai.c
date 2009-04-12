/* $Header: /afs/athena.mit.edu/astaff/project/moiradev/repository/moira/webmoira/moirai.c,v 1.3 2006-08-22 17:36:27 zacheiss Exp $ */
#include "mit_moira_MoiraConnectInternal.h"
#include <moira.h>
#include <mr_et.h>
#include <malloc.h>
#include <string.h>
#include <stdio.h>

static void throwMoiraException(JNIEnv *env, int code)
{
    char buffer[1024];
    jmethodID mid;
    jobject tothrow;
    jclass IOE;
    jstring jmess;

    IOE = (*env)->FindClass(env, "mit/moira/MoiraException");
    if (IOE == NULL) {
	fprintf(stderr, "moirai: No Class\n");
	goto die;
    }
    mid = (*env)->GetMethodID(env, IOE, "<init>", "(Ljava/lang/String;I)V");
    if (mid == NULL) {
	fprintf(stderr, "moirai: No Method\n");
	goto die;
    }
    sprintf(buffer, "%s", error_message(code));
    jmess = (*env)->NewStringUTF(env, buffer);
    if (jmess == NULL) {
	fprintf(stderr, "Cannot get new string\n");
	goto die;
    }
    tothrow = (*env)->NewObject(env, IOE, mid, jmess, (jint) code);
    if (tothrow == NULL) {
	fprintf(stderr, "moirai: No Throw\n");
	goto die;
    }
    (*env)->Throw(env, (jthrowable) tothrow);
    return;
 die:
    abort();
    return;
}

static void throwMoiraExceptionMess(JNIEnv *env, char *mess)
{
    jclass IOE = (*env)->FindClass(env, "mit/moira/MoiraException");
    if (IOE == NULL) abort();
    (*env)->ThrowNew(env, IOE, mess);
    return;
}

JNIEXPORT void JNICALL Java_mit_moira_MoiraConnectInternal_connect(JNIEnv *env,
							 jclass Class, jstring server) {
    int status;
    char buffer[1024];
    const char *aserver = (*env)->GetStringUTFChars(env, server, 0);
    if (strlen(aserver) > sizeof(buffer)) abort();
    strcpy(buffer, aserver);
    (*env)->ReleaseStringUTFChars(env, server, aserver);
    status = mr_connect(buffer);
    if (status != MR_SUCCESS) throwMoiraException(env, status);
    status = mr_version(2);
    if (status != MR_SUCCESS) {
	if (status == MR_VERSION_LOW) return; /* This is OK */
	else {
	    mr_disconnect();
	    throwMoiraException(env, status);
	}
    }
    return;
}

JNIEXPORT void JNICALL Java_mit_moira_MoiraConnectInternal_proxy(JNIEnv *env,
						       jclass Class,
						       jstring jUser) {
    int status;
    char buffer[1024];
    const char *user = (*env)->GetStringUTFChars(env, jUser, 0);
    if (strlen(user) > sizeof(buffer)) abort();
    strcpy(buffer, user);
    (*env)->ReleaseStringUTFChars(env, jUser, user);
    status = mr_proxy(buffer, "Java");
    if (status != MR_SUCCESS) throwMoiraException(env, status);
    return;
}


JNIEXPORT void JNICALL Java_mit_moira_MoiraConnectInternal_auth(JNIEnv *env,
						      jclass Class) {
    int status;
    status = mr_krb5_auth("JavaInterface");
    if (status != MR_SUCCESS) throwMoiraException(env, status);
    return;
}

JNIEXPORT void JNICALL Java_mit_moira_MoiraConnectInternal_disconnect
    (JNIEnv *env, jclass Class) {
    mr_disconnect();
    return;
}
	
typedef struct ArgBlock {
    int alloccount;
    int count;
    char **stringval;
} ArgBlock;

typedef struct ListBlock {
    int alloccount;
    int count;
    ArgBlock **Args;
} ListBlock;

static jobjectArray ArgBlockToJavaArray(JNIEnv *env, ArgBlock *argb);
static jobjectArray ListBlockToJavaArray(JNIEnv *env, ListBlock *lb);

extern int mr_query(char *name, int argc, char **argv,
		int (*proc)(int, char **, void *), void *hint);
static ListBlock *NewListBlock();
static ArgBlock *NewArgBlock();
static void FreeListBlock(ListBlock *);
static int StashResults(int argc, char **argv, void *callback);
static char **ConvertJavaArgs(JNIEnv *env, jobjectArray jargs);
static void FreeArgs(char **args);

JNIEXPORT jobjectArray JNICALL Java_mit_moira_MoiraConnectInternal_mr_1query
    (JNIEnv *env, jclass Class, jstring jcommand, jobjectArray jargs) {
    ListBlock *listblock = NewListBlock();
    jobjectArray retval;
    int status;
    const char *command;
    char icommand[1024];
    char **args = ConvertJavaArgs(env, jargs);
    if (args == NULL) return (NULL); /* It probably thru an exception */
    command = (*env)->GetStringUTFChars(env, jcommand, 0);
    strncpy(icommand, command, sizeof(icommand));

    status = mr_query(icommand, (*env)->GetArrayLength(env, jargs), args, 
			 StashResults, listblock);
    FreeArgs(args);			/* Don't need them anymore */
    if (status != MR_SUCCESS) {
	FreeListBlock(listblock);
	throwMoiraException(env, status);
	return (0);
    }
    /*    if (listblock->count == 0) { / * No such list or empty list * /
	FreeListBlock(listblock);
	throwMoiraExceptionMess(env, "No Such List or Empty List");
	return(0);
    } */

    if (listblock->count == 0) return (0);

    retval = ListBlockToJavaArray(env, listblock);
    FreeListBlock(listblock);
    return (retval);

}

static ArgBlock *NewArgBlock() {
    ArgBlock *argb = (ArgBlock *)malloc(sizeof(ArgBlock));
    argb->alloccount = 10;
    argb->stringval = (char **) malloc(10 * sizeof(char *));
    argb->count = 0;
    return (argb);
}

static void AddArg(ArgBlock *argb, char *arg) {
    int i;
    if (argb->alloccount <= (argb->count + 1)) {
	char **st = malloc((2 * argb->alloccount) * sizeof(char *));
	argb->alloccount *= 2;
	for (i = 0; i < argb->count; i++)
	    st[i] = argb->stringval[i];
	free(argb->stringval);
	argb->stringval = st;
    }
    argb->stringval[argb->count++] = arg;
}

static ListBlock *NewListBlock() {
    ListBlock *list = (ListBlock *)malloc(sizeof(ListBlock));
    list->alloccount = 10;
    list->Args = (ArgBlock **) malloc(10 * sizeof(ArgBlock *));
    list->count = 0;
    return (list);
}

static void FreeArgBlock(ArgBlock *argb) {
    int i;
    for (i = 0; i < argb->count; i++) free(argb->stringval[i]);
    free(argb->stringval);
    free(argb);
}

static void FreeListBlock(ListBlock *list) {
    int i;
    for (i = 0; i < list->count; i++) FreeArgBlock(list->Args[i]);
    free(list->Args);
    free(list);
}

static jobjectArray ArgBlockToJavaArray(JNIEnv *env, ArgBlock *argb) {
    jobjectArray retval;
    int i;
    retval = (*env)->NewObjectArray(env, argb->count,
				    (*env)->FindClass(env, "java/lang/String"),
				    NULL);
    for (i = 0; i < argb->count; i++) {
	(*env)->SetObjectArrayElement(env, retval, i,
				      (*env)->NewStringUTF(env, argb->stringval[i]));
    }
    return (retval);
}

static jobjectArray ListBlockToJavaArray(JNIEnv *env, ListBlock *lb) {
    jobjectArray retval;
    int i;
    retval = (*env)->NewObjectArray(env, lb->count,
				    (*env)->FindClass(env, "java/lang/Object"),
				    NULL);
    for (i = 0; i < lb->count; i++)
	(*env)->SetObjectArrayElement(env, retval, i,
				      ArgBlockToJavaArray(env, lb->Args[i]));
    return (retval);
}

static int StashResults(int argc, char **argv, void *callback) {
    ListBlock *list = (ListBlock *) callback;
    ArgBlock *argb;
    char *arg;
    int i;

    /* printf("DEBUG: StashResults argc = %d\n", argc);
     * for (i = 0; i < argc; i++)
     * printf("DEBUG: StashResults argv[%d] = %s\n", i, argv[i]);
     */

    while (list->alloccount <= (list->count + 1)) {
	ArgBlock **args = (ArgBlock **) malloc(list->alloccount * 2 * sizeof(char *));
	list->alloccount = list->alloccount * 2;
	for(i = 0; i < list->count; i++) {
	    args[i] = list->Args[i];
	}
	free(list->Args);
	list->Args = args;
    }

    argb = NewArgBlock();

    for (i = 0; i < argc; i++) {
	arg = (char *) malloc(strlen(argv[i]) + 1);
	strcpy(arg, argv[i]);
	AddArg(argb, arg);
    }
    list->Args[list->count++] = argb;
    return (0);
}

static char **ConvertJavaArgs(JNIEnv *env, jobjectArray jargs) {
    char **retval;
    int i, j;
    const char *iarg;
    int length = (*env)->GetArrayLength(env, jargs);
    if (length == 0) {		/* Does this happen in a non-error situation? */
	retval = (char **) malloc(sizeof (char *));
	retval[0] = NULL;
	return(retval);
    }
    retval = (char **) malloc((length + 1) * sizeof(char *));
    for (i = 0; i < length; i++) {
	jobject jarg = (*env)->GetObjectArrayElement(env, jargs, i);
	if ((*env)->ExceptionOccurred(env)) {
	    for (j = 0; j < i; j++) free(retval[j]);
	    free (retval);
	    return (NULL);
	}
	iarg = (*env)->GetStringUTFChars(env, jarg, 0);
	if ((*env)->ExceptionOccurred(env)) {
	    for (j = 0; j < i; j++) free(retval[j]);
	    free (retval);
	    return (NULL);
	}
	retval[i] = malloc(strlen(iarg) + 1);
	strcpy(retval[i], iarg);
	(*env)->ReleaseStringUTFChars(env, jarg, iarg);
    }
    retval[length] = NULL;
    return (retval);
}

static void FreeArgs(char **args) {
    int i;
    i = 0;
    while (args[i] != NULL) free(args[i++]);
    free(args);
}

