#include <stdlib.h>
#include <stdio.h>

#include "webauth.h"
#include "webauthtest.h"

#define BUFSIZE 4096
#define MAX_ATTRS 128

void
usage()
{
    printf("usage: krb5_test {username} {password} {keytab}\n");
    exit(1);
}
int main(int argc, char *argv[])
{
    int s;
    WEBAUTH_KRB5_CTXT *c;
    TEST_VARS;
    char *username, *password, *keytab_path;
    char *cp;
    unsigned char *sa;
    int salen;
    unsigned char *tgt;
    int tgtlen;
    time_t expiration;

    if (argc != 4) {
        usage();
    }

    username = argv[1];
    password = argv[2];
    keytab_path = argv[3];

    START_TESTS(12);

    c = webauth_krb5_new();
    TEST_OK(c != NULL);

    s = webauth_krb5_init_via_password(c, username, password, 
                                       keytab_path, NULL);

    TEST_OK2(WA_ERR_NONE, s);

    sa = NULL;
    s = webauth_krb5_mk_req(c, "lichen.stanford.edu", "host", &sa, &salen);
    TEST_OK2(WA_ERR_NONE, s);

    s = webauth_krb5_rd_req(c, sa, salen, keytab_path, &cp);
    TEST_OK2(WA_ERR_NONE, s);
    if (cp) {
        free(cp);
    }

    if (sa != NULL) {
        free(sa);
    }

    tgt = NULL;
    s = webauth_krb5_export_tgt(c, &tgt, &tgtlen, &expiration);
    TEST_OK2(WA_ERR_NONE, s);

    s = webauth_krb5_free(c, 1);
    TEST_OK2(WA_ERR_NONE, s);
        
    if (tgt != NULL) {
        c = webauth_krb5_new();
        TEST_OK(c != NULL);
            
        s = webauth_krb5_init_via_tgt(c, tgt, tgtlen, NULL);
        free(tgt);
        TEST_OK2(WA_ERR_NONE, s);

        s = webauth_krb5_free(c, 1);
        TEST_OK2(WA_ERR_NONE, s);
    }

    c = webauth_krb5_new();
    TEST_OK(c != NULL);
            
    s = webauth_krb5_init_via_keytab(c, keytab_path, NULL);
    TEST_OK2(WA_ERR_NONE, s);

    s = webauth_krb5_free(c, 0);
    TEST_OK2(WA_ERR_NONE, s);

    END_TESTS;
    exit(NUM_FAILED_TESTS ? 1 : 0);
}
