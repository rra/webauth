/* 
**  mod_webauth.c -- Apache sample webauth module
**  [Autogenerated via ``apxs -n webauth -g'']
**
**  To play with this sample module first compile it into a
**  DSO file and install it into Apache's modules directory 
**  by running:
**
**    $ apxs -c -i mod_webauth.c
**
**  Then activate it in Apache's httpd.conf file for instance
**  for the URL /webauth in as follows:
**
**    #   httpd.conf
**    LoadModule webauth_module modules/mod_webauth.so
**    <Location /webauth>
**    SetHandler webauth
**    </Location>
**
**  Then after restarting Apache via
**
**    $ apachectl restart
**
**  you immediately can request the URL /webauth and watch for the
**  output of this module. This can be achieved for instance via:
**
**    $ lynx -mime_header http://localhost/webauth 
**
**  The output should be similar to the following one:
**
**    HTTP/1.1 200 OK
**    Date: Tue, 31 Mar 1998 14:42:22 GMT
**    Server: Apache/1.3.4 (Unix)
**    Connection: close
**    Content-Type: text/html
**  
**    The sample page from mod_webauth.c
*/ 

#include "mod_webauth.h"

/*
 * initialized only in mod_webauth_child_init
 */
WEBAUTH_KEYRING *mwa_g_ring;
MWA_SERVICE_TOKEN *mwa_g_service_token;


/*
 * remove a string from the end of another string
 */
static void
strip_end(char *c, char *t)
{
    char *p;
    if (c != NULL) {
        p = ap_strstr(c, t);
        if (p != NULL)
            *p = '\0';
    }
}

static int 
die(const char *message, server_rec *s)
{
    if (s) {
        ap_log_error(APLOG_MARK, APLOG_ERR, 0, s,
                     "mod_webauth: fatal error: %s", message);
    }
    printf("mod_webauth: fatal error: %s\n", message);
    exit(1);
}

/*
 * called after config has been loaded in parent process
 */
static int
mod_webauth_init(apr_pool_t *pconf, apr_pool_t *plog,
                 apr_pool_t *ptemp, server_rec *s)
{
    MWA_SCONF *sconf;
    sconf = (MWA_SCONF*)ap_get_module_config(s->module_config,
                                                 &webauth_module);

    ap_log_error(APLOG_MARK, APLOG_ERR, 0, s, "mod_webauth: initializing");

#define CHECK_DIR(field,dir) if (sconf->field == NULL) \
             die(apr_psprintf(ptemp, "directive %s must be set", dir), s)

    CHECK_DIR(login_url, CD_LoginURL);
    CHECK_DIR(keyring_path, CD_Keyring);
    CHECK_DIR(webkdc_url, CD_WebKDCURL);
    CHECK_DIR(keytab_path, CD_Keytab);
    CHECK_DIR(webkdc_principal, CD_WebKDCPrincipal);
    CHECK_DIR(webkdc_principal, CD_WebKDCPrincipal);
    CHECK_DIR(st_cache_path, CD_ServiceTokenCache);

#undef CHECK_DIR

    ap_add_version_component(pconf, WEBAUTH_VERSION);

    ap_log_error(APLOG_MARK, APLOG_ERR, 0, s, "mod_webauth: initialized");

    return OK;
}

/*
 * called once per-child
 */
static void
mod_webauth_child_init(apr_pool_t *p, server_rec *s)
{
    MWA_SCONF *sconf;
    int status;

    sconf = (MWA_SCONF*)ap_get_module_config(s->module_config,
                                             &webauth_module);

    /* attempt to open up keyring */
    status = webauth_keyring_read_file(sconf->keyring_path, &mwa_g_ring);
    if (status != WA_ERR_NONE) {
        die(apr_psprintf(p, 
                 "mod_webauth: webauth_keyring_read_file(%s) failed: %s (%d)",
                         sconf->keyring_path, webauth_error_message(status), 
                         status), s);
    } else {
        /* FIXME: should probably make sure we have at least one
           valid (not expired/postdated) key in the ring */
    }

    /* FIXME: should probably attempt to read_service_token_cache */
    mwa_g_service_token = NULL;
}

/*
**
**  per-server configuration structure handling
**
*/

static void *
config_server_create(apr_pool_t *p, server_rec *s)
{
    MWA_SCONF *sconf;

    sconf = (MWA_SCONF*)apr_pcalloc(p, sizeof(MWA_SCONF));

    /* init defaults */
    sconf->secure_cookie = DF_SecureCookie;
    sconf->token_max_ttl = DF_TokenMaxTTL;
    sconf->subject_auth_type = DF_SubjectAuthType;
    sconf->strip_url = DF_StripURL;
    return (void *)sconf;
}

static void *
config_dir_create(apr_pool_t *p, char *path)
{
    MWA_DCONF *dconf;
    dconf = (MWA_DCONF*)apr_pcalloc(p, sizeof(MWA_DCONF));
    /* init defaults */

    return (void *)dconf;
}


#define MERGE_PTR(field) \
    conf->field = (oconf->field != NULL) ? oconf->field : bconf->field

#define MERGE_INT(field) \
    conf->field = oconf->field ? oconf->field : bconf->field

static void *
config_server_merge(apr_pool_t *p, void *basev, void *overv)
{
    MWA_SCONF *conf, *bconf, *oconf;

    conf = (MWA_SCONF*) apr_pcalloc(p, sizeof(MWA_SCONF));
    bconf = (MWA_SCONF*) basev;
    oconf = (MWA_SCONF*) overv;

    /* secure_cookie is 1 by default, so we need to check if
       it was explicitly set in the override */
    
    conf->secure_cookie = oconf->secure_cookie_ex ?
        oconf->secure_cookie : bconf->secure_cookie;

    conf->token_max_ttl = oconf->token_max_ttl_ex ?
        oconf->token_max_ttl : bconf->token_max_ttl;

    conf->subject_auth_type = oconf->subject_auth_type_ex ?
        oconf->subject_auth_type : bconf->subject_auth_type;

    conf->strip_url = oconf->strip_url_ex ?
        oconf->strip_url : bconf->strip_url;

    conf->extra_redirect = oconf->extra_redirect_ex ?
        oconf->extra_redirect : bconf->extra_redirect;

    conf->debug = oconf->debug_ex ? oconf->debug : bconf->debug;

    MERGE_PTR(webkdc_url);
    MERGE_PTR(webkdc_principal);
    MERGE_PTR(login_url);
    MERGE_PTR(failure_url);
    MERGE_PTR(keyring_path);
    MERGE_PTR(keytab_path);
    MERGE_PTR(st_cache_path);
    MERGE_PTR(var_prefix);
    return (void *)conf;
}

static void *
config_dir_merge(apr_pool_t *p, void *basev, void *overv)
{
    MWA_DCONF *conf, *bconf, *oconf;

    conf = (MWA_DCONF*) apr_pcalloc(p, sizeof(MWA_DCONF));
    bconf = (MWA_DCONF*) basev;
    oconf = (MWA_DCONF*) overv;

    conf->force_login = oconf->force_login_ex ? 
        oconf->force_login : bconf->force_login;

    MERGE_INT(app_token_lifetime);
    MERGE_INT(inactive_expire);
    MERGE_PTR(return_url);
    return (void *)conf;
}

#undef MERGE_PTR
#undef MERGE_INT

/* The sample content handler */
static int 
handler_hook(request_rec *r)
{
    if (strcmp(r->handler, "webauth")) {
        return DECLINED;
    }
    r->content_type = "text/html";      

    if (!r->header_only)
        ap_rputs("The sample page from mod_webauth.c\n", r);
    return OK;
}

static WEBAUTH_ATTR_LIST *
parse_app_token(char *token, MWA_REQ_CTXT *rc)
{
    WEBAUTH_ATTR_LIST *alist;
    int blen, status;
    const char *tt;

    ap_unescape_url(token);
    blen = apr_base64_decode(token, token);

    /* parse the token, TTL is zero because app-tokens don't have ttl,
     * just expiration
     */
    status = webauth_token_parse(token, blen, 0, mwa_g_ring, &alist);
    if (status != WA_ERR_NONE) {
        mwa_log_webauth_error(rc->r, status, NULL,
                              "parse_app_token", "webauth_token_parse");
        return NULL;
    }

    /* make sure its an app-token */
    tt = mwa_get_str_attr(alist, WA_TK_TOKEN_TYPE, rc->r,
                          "check_cookie", NULL);
    if (tt == NULL || strcmp(tt, WA_TT_APP) != 0) {
        ap_log_error(APLOG_MARK, APLOG_ERR, 0, rc->r->server,
                     "mod_webauth: check_cookie: token type(%s) not (%s)",
                     tt ? tt : "(null)", WA_TT_APP);
        webauth_attr_list_free(alist);
        return NULL;
    }
    return alist;
}

/*
 * check cookie for valid app-token. If an epxired one is found,
 * do a Set-Cookie (in fixups) to blank it out.
 */
static char *
check_cookie(MWA_REQ_CTXT *rc)
{
    const char *c;
    char *cs, *ce, *cval, *sub;
    WEBAUTH_ATTR_LIST *alist;

    c = apr_table_get(rc->r->headers_in, "Cookie");
    if (c == NULL) 
        return NULL;

    cs = ap_strstr(c, AT_COOKIE_NAME_EQ);
    if (cs == NULL) {
        return NULL;
    } else {
        cs += sizeof(AT_COOKIE_NAME_EQ)-1;
    }

    ce = ap_strchr(cs, ';');

    if (ce == NULL) {
        cval = apr_pstrdup(rc->r->pool, cs);
    } else {
        cval = apr_pstrmemdup(rc->r->pool, cs, ce-cs);
    }

    sub = NULL;

    alist = parse_app_token(cval, rc);
    if (alist != NULL) {
        /* pull out subject */
        const char *tsub = 
            mwa_get_str_attr(alist, WA_TK_SUBJECT, rc->r, 
                             "check_cookie", NULL);
        if (tsub != NULL) {
            sub = apr_pstrdup(rc->r->pool, tsub);
        }
        webauth_attr_list_free(alist);
    }

    if (sub == NULL) {
        /* we coudn't use the cookie, lets set it up to be nuked */
        char *cookie = apr_psprintf(rc->r->pool,
                                    "%s=; path=/; expires=%s;%s",
                                    AT_COOKIE_NAME,
                                    "Thu, 26-Mar-1998 00:00:01 GMT",
                                    rc->sconf->secure_cookie ? "secure" : "");
        ap_log_error(APLOG_MARK, APLOG_ERR, 0, rc->r->server,
                     "mod_webauth: nuking cookie(%s): (%s)\n", 
                     AT_COOKIE_NAME, cookie);
        mwa_setn_note(rc->r, N_APP_COOKIE, cookie);
        return NULL;
    }

    if (sub != NULL) {
        ap_log_error(APLOG_MARK, APLOG_ERR, 0, rc->r->server,
                     "mod_webauth: found valid %s cookie for (%s)", 
                     AT_COOKIE_NAME,
                     sub);

    }
    return sub;
}

WEBAUTH_KEY *
get_session_key(char *token, MWA_REQ_CTXT *rc)
{
    WEBAUTH_ATTR_LIST *alist;
    WEBAUTH_KEY *key;
    int status, i , klen;

    alist = parse_app_token(token, rc);

    if (alist == NULL)
        return NULL;

    /* pull out session key */
    status = webauth_attr_list_find(alist, WA_TK_SESSION_KEY, &i);
    if (i == -1) {
        ap_log_error(APLOG_MARK, APLOG_ERR, 0, rc->r->server, 
                    "mod_webauth: check_url: can't find session key in token");
        webauth_attr_list_free(alist);
        return NULL;
    }

    klen = alist->attrs[i].length;

    if (klen != WA_AES_128 && 
        klen != WA_AES_192 &&
        klen != WA_AES_256) {
        ap_log_error(APLOG_MARK, APLOG_ERR, 0, rc->r->server, 
                     "mod_webauth: get_session_key: invalid key length: %d",
                     klen);
        return NULL;
    }

    key = (WEBAUTH_KEY*) apr_palloc(rc->r->pool, sizeof(WEBAUTH_KEY));
    key->type = WA_AES_KEY;
    key->data = (unsigned char*) apr_palloc(rc->r->pool, klen);
    memcpy(key->data, alist->attrs[i].value, klen);
    key->length = klen;

    webauth_attr_list_free(alist);

    return key;
}

static char *
validate_krb5_sad(WEBAUTH_ATTR_LIST *alist, MWA_REQ_CTXT *rc)

{
    WEBAUTH_KRB5_CTXT *ctxt;
    int status, i;
    char *principal, *subject;

    status = webauth_attr_list_find(alist, WA_TK_SUBJECT_AUTH_DATA, &i);
    if (i == -1) {
        ap_log_error(APLOG_MARK, APLOG_ERR, 0, rc->r->server, 
                     "mod_webauth: validate_krb5_sad: "
                     "can't find subject auth data");
        return NULL;
    }

    ctxt = mwa_get_webauth_krb5_ctxt(rc->r, "validate_krb5_sad");
    if (ctxt == NULL)
        return NULL;

    status = webauth_krb5_rd_req(ctxt,
                                 alist->attrs[i].value,
                                 alist->attrs[i].length,
                                 rc->sconf->keytab_path,
                                 &principal);

    webauth_krb5_free(ctxt);

    if (status != WA_ERR_NONE) {
        mwa_log_webauth_error(rc->r, status, ctxt, "validate_krb5_sad",
                              "webauth_krb5_rd_req");
        return NULL;
    }

    subject = apr_pstrcat(rc->r->pool, "krb5:", principal, NULL);
    free(principal);
    return subject;
}

static void
make_app_token(char *subject, time_t expiration_time, MWA_REQ_CTXT *rc)
{
    WEBAUTH_ATTR_LIST *alist;
    time_t curr = time(NULL);
    char *token, *btoken, *cookie;
    int tlen, olen, status;

    alist = webauth_attr_list_new(10);
    if (alist == NULL) {
        ap_log_error(APLOG_MARK, APLOG_ERR, 0, rc->r->server,
                     "mod_webauth: make_app_token: "
                     "webauth_attr_list_new failed");
        return;
    }

    if (rc->dconf->app_token_lifetime) {
        expiration_time = curr + rc->dconf->app_token_lifetime;
    }

    webauth_attr_list_add_str(alist, WA_TK_TOKEN_TYPE, WA_TT_APP, 0, 
                              WA_F_NONE);
    webauth_attr_list_add_str(alist, WA_TK_SUBJECT, subject, 0, WA_F_NONE);
    webauth_attr_list_add_time(alist, WA_TK_EXPIRATION_TIME,
                               expiration_time, WA_F_NONE);

    webauth_attr_list_add_time(alist, WA_TK_CREATION_TIME, curr, WA_F_NONE);
    
    /* FIXME: handle it/lt app_token_lifetime, inactive/etc */

    tlen = webauth_token_encoded_length(alist);
    token = (char*)apr_palloc(rc->r->pool, tlen);

    status = webauth_token_create(alist, curr, token, &olen, tlen, mwa_g_ring);

    webauth_attr_list_free(alist);

    if (status != WA_ERR_NONE) {
        mwa_log_webauth_error(rc->r, status, NULL, "make_app_token",
                              "webauth_token_create");
        return;
    }

    btoken = (char*) apr_palloc(rc->r->pool, apr_base64_encode_len(olen));
    apr_base64_encode(btoken, token, olen);

    cookie = apr_psprintf(rc->r->pool,
                          "%s=%s; path=/;%s",
                          AT_COOKIE_NAME,
                          btoken,
                          rc->sconf->secure_cookie ? "secure" : "");
    ap_log_error(APLOG_MARK, APLOG_ERR, 0, rc->r->server,
                 "mod_webauth: make_app_token setting cookie(%s): (%s)\n", 
                 AT_COOKIE_NAME, cookie);
    mwa_setn_note(rc->r, N_APP_COOKIE, cookie);
}


static char *
handle_id_token(WEBAUTH_ATTR_LIST *alist, MWA_REQ_CTXT *rc)
{
    char *subject;
    int status;
    const char *sa = mwa_get_str_attr(alist, WA_TK_SUBJECT_AUTH,
                                      rc->r, "handle_id_token", NULL);
    if (sa == NULL ) {
        /* nothing, we already logged an error */
        return NULL;
    }

    if (strcmp(sa, WA_SA_KRB5) == 0) {
        subject = validate_krb5_sad(alist, rc);
    } else if (strcmp(sa, WA_SA_WEBKDC) == 0) {
        /* subject is all set */
        const char *tsub = mwa_get_str_attr(alist, WA_TK_SUBJECT,
                                            rc->r, "handle_id_token", NULL);
        subject = apr_pstrdup(rc->r->pool, tsub);
    } else {
        /* FIXME: HUM.... */
        ap_log_error(APLOG_MARK, APLOG_ERR, 0, rc->r->server,
                     "mod_webauth: handle_id_token: "
                     "unknown subject auth type: %s", sa);
        subject = NULL;
    }
        
    if (subject != NULL) {
        time_t expiration_time;
        /* wheeee! create an app-token! */

        status = webauth_attr_list_get_time(alist, WA_TK_EXPIRATION_TIME,
                                            &expiration_time,
                                            WA_F_NONE);
        if (status != WA_ERR_NONE) {
            ap_log_error(APLOG_MARK, APLOG_ERR, 0, rc->r->server,
                         "mod_webauth: parse_returned_token: "
                         "can't get expiration time from id token token");
        } else {
            ap_log_error(APLOG_MARK, APLOG_ERR, 0, rc->r->server,
                         "mod_webauth: parse_returned_token: "
                         "got subject(%s) from id token", subject);
            make_app_token(subject, expiration_time, rc);
        }
    } else {
        /* FIXME: everyone else should have logged something, right? */
    }
    return subject;
}

static char *
parse_returned_token(char *token, WEBAUTH_KEY *key, MWA_REQ_CTXT *rc)
{
    WEBAUTH_ATTR_LIST *alist;
    int blen, status;
    const char *token_type;
    char *subject;
    static const char *mwa_func = "parse_returned_token";
    subject = NULL;

    /* if we successfully parse an id-token, write out new webauth_at cookie */
    ap_unescape_url(token);
    blen = apr_base64_decode(token, token);

    status = webauth_token_parse_with_key(token, blen, 
                                          rc->sconf->token_max_ttl, 
                                          key, &alist);

    if (status != WA_ERR_NONE) {
        mwa_log_webauth_error(rc->r, status, NULL, mwa_func,
                              "webauth_token_parse_with_key");
        return NULL;
    }

    /* make sure its an app-token */
    token_type = mwa_get_str_attr(alist, WA_TK_TOKEN_TYPE, rc->r, 
                                  mwa_func, NULL);
    if (token_type == NULL) {
        webauth_attr_list_free(alist);
        return NULL;
    }

    if (strcmp(token_type, WA_TT_ID) == 0) {

        subject = handle_id_token(alist, rc);

    } else if (strcmp(token_type, WA_TT_PROXY) == 0) {

        ap_log_error(APLOG_MARK, APLOG_ERR, 0, rc->r->server,
                     "mod_webauth: %s: parsed a proxy token", mwa_func);

    } else if (strcmp(token_type, WA_TT_ERROR) == 0) {

        /* FIXME: 
         *   special handling for some errors code:
         *
         *    if ec = bad/expired-service-token, we need to
         *    toss our in memory (and potentiall file cached) service
         *    token,
         *
         *    stale request?
         *
         *    other error codes? probably need to detect a rejected
         *    request for a proxy-token
         *
         */

        ap_log_error(APLOG_MARK, APLOG_ERR, 0, rc->r->server,
                     "mod_webauth: %s: parsed an error token", mwa_func);

    } else {
        ap_log_error(APLOG_MARK, APLOG_ERR, 0, rc->r->server,
                     "mod_webauth: %s: unhandled token type(%s)",
                     mwa_func,
                     token_type);
    }
    webauth_attr_list_free(alist);

    return subject;
}

/*
 * check to see if we got passed WEBAUTHR and WEBAUTHS
 */
static char *
check_url(MWA_REQ_CTXT *rc)
{
    char *subject, *wr, *ws;
    WEBAUTH_KEY *key = NULL;

    subject = NULL;

    wr = mwa_remove_note(rc->r, N_WEBAUTHR);
    if (wr == NULL) {
        return NULL;
    }

    ap_log_error(APLOG_MARK, APLOG_ERR, 0, rc->r->server,
                 "mod_webauth: check_url: found wr(%s)", wr);

    /* see if we have WEBAUTHS, which has the session key to use */
    ws = mwa_remove_note(rc->r, N_WEBAUTHS);

    if (ws != NULL) {
        /* don't have to free key, its allocated from a pool */
        key = get_session_key(ws, rc);
        if (key == NULL)
            return NULL;
        subject = parse_returned_token(wr, key, rc);
    } else {
        MWA_SERVICE_TOKEN *st = mwa_get_service_token(rc);
        if (st != NULL)
            subject = parse_returned_token(wr, &st->key, rc);
    }
    return subject;
}

static char *
make_return_url(MWA_REQ_CTXT *rc)
{
    char port[32];
    const char *scheme;
    const char *uri = rc->r->unparsed_uri;

    /* use explicit return_url if there is one */
    if (rc->dconf->return_url) {
        if (rc->dconf->return_url[0] != '/')
            return rc->dconf->return_url;
        else 
            uri = rc->dconf->return_url;
    }

    scheme = ap_run_http_method(rc->r);

    if (ap_is_default_port(rc->r->server->port, rc->r)) {
        port[0] = '\0';
    } else {
        sprintf(port, ":%d", rc->r->server->port);
    }

    return apr_pstrcat(rc->r->pool,
                       scheme, "://",
                       rc->r->server->server_hostname, 
                       port, 
                       uri,
                       NULL);
}

static int
redirect_request_token(MWA_REQ_CTXT *rc)
{
    MWA_SERVICE_TOKEN *st;
    WEBAUTH_ATTR_LIST *alist;
    char *redirect_url, *return_url;
    unsigned char *token, *btoken;
    int tlen, olen, status;
    time_t curr = time(NULL);

    st = mwa_get_service_token(rc);
    if (st == NULL) {
        ap_log_error(APLOG_MARK, APLOG_ERR, 0, rc->r->server,
                     "mod_webauth: redirect_request_token: "
                     "no service token, denying request");
        return HTTP_UNAUTHORIZED;
    }

    alist = webauth_attr_list_new(10);
    if (alist == NULL) {
        ap_log_error(APLOG_MARK, APLOG_ERR, 0, rc->r->server,
                     "mod_webauth: redirect_request_token: "
                     "webauth_attr_list_new failed");
        return HTTP_UNAUTHORIZED;
    }

    webauth_attr_list_add_str(alist, WA_TK_TOKEN_TYPE, WA_TT_REQUEST, 0, 
                              WA_F_NONE);

    webauth_attr_list_add_time(alist, WA_TK_CREATION_TIME, curr, WA_F_NONE);

    if (st->app_state != NULL) {
        webauth_attr_list_add(alist, WA_TK_APP_STATE, 
                              st->app_state, st->app_state_len,
                              WA_F_NONE);
    }

    /* FIXME: hardcoded for now */
    webauth_attr_list_add_str(alist, WA_TK_REQUEST_REASON,
                              "na", 0, WA_F_NONE);

    webauth_attr_list_add_str(alist, WA_TK_REQUESTED_TOKEN_TYPE, 
                              WA_TT_ID, 0, WA_F_NONE);

    webauth_attr_list_add_str(alist, WA_TK_SUBJECT_AUTH,
                              rc->sconf->subject_auth_type, 0, WA_F_NONE);

    return_url = make_return_url(rc);

    /* never let return URL have  ;WEBAUTHR=...;;WEBUTHS=...; on the
       end of it, that could get ugly... */
    strip_end(return_url, WEBAUTHR_MAGIC);

    webauth_attr_list_add_str(alist, WA_TK_RETURN_URL, return_url,
                              0, WA_F_NONE);

    ap_log_error(APLOG_MARK, APLOG_ERR, 0, rc->r->server,
                 "mod_webauth: redirect_requst_token: return_url(%s)",
                 return_url);

    tlen = webauth_token_encoded_length(alist);
    token = (char*)apr_palloc(rc->r->pool, tlen);

    status = webauth_token_create_with_key(alist, curr,
                                           token, &olen, tlen, &st->key);

    webauth_attr_list_free(alist);

    if (status != WA_ERR_NONE) {
        ap_log_error(APLOG_MARK, APLOG_ERR, 0, rc->r->server,
                     "mod_webauth: redirect_request_token: "
                     "type(%d) length(%d)", st->key.type, st->key.length);


        mwa_log_webauth_error(rc->r, status, NULL, "redirect_request_token",
                              "webauth_token_create_with_key");
        return HTTP_UNAUTHORIZED;
    }

    btoken = (char*) apr_palloc(rc->r->pool, apr_base64_encode_len(olen));
    apr_base64_encode(btoken, token, olen);

    redirect_url = apr_pstrcat(rc->r->pool,
                               rc->sconf->login_url,
                               "?RT=", btoken,
                               ";ST=", st->token,
                               NULL);
    
    apr_table_setn(rc->r->err_headers_out, "Location", redirect_url);
                               
    ap_log_error(APLOG_MARK, APLOG_ERR, 0, rc->r->server,
                 "mod_webauth: redirect_requst_token: redirect(%s)",
                 redirect_url);

    return HTTP_MOVED_TEMPORARILY;
}


static int
extra_redirect(MWA_REQ_CTXT *rc)
{
    char *redirect_url;
    
    redirect_url = make_return_url(rc);
    /* always strip extra-redirect URL */
    strip_end(redirect_url, WEBAUTHR_MAGIC);

    apr_table_setn(rc->r->err_headers_out, "Location", redirect_url);
                               
    ap_log_error(APLOG_MARK, APLOG_ERR, 0, rc->r->server,
                 "mod_webauth: extra_redirect: redirect(%s)",
                 redirect_url);

    return HTTP_MOVED_TEMPORARILY;
}

static int 
check_user_id_hook(request_rec *r)
{
    const char *at = ap_auth_type(r);
    const char *subject, *cookie;
    int in_url = 0;
    MWA_REQ_CTXT rc;
   
    rc.r = r;

    rc.dconf = (MWA_DCONF*)ap_get_module_config(r->per_dir_config,
                                                &webauth_module);

    rc.sconf = (MWA_SCONF*)ap_get_module_config(r->server->module_config,
                                                &webauth_module);

    ap_log_error(APLOG_MARK, APLOG_ERR, 0, r->server,
                 "mod_webauth: in check_user_id hook");

    if ((at == NULL) || (strcmp(at, "WebAuth") != 0)) {
        return DECLINED;
    }

    /* first check if we've already validated the user */
    subject = mwa_get_note(r, N_SUBJECT);

    if (subject == NULL) {
        /* next, check for valid app-token cookie */
        subject = check_cookie(&rc);

        if (subject == NULL) {
            /* if no valid app token, look for WEBAUTHR in url */
            subject = check_url(&rc);

            if (subject != NULL)
                in_url = 1;
        }
        /* stick it in note for future reference */
        if (subject != NULL) 
            mwa_setn_note(r, N_SUBJECT, subject);
    }

    /* see if we have to update our cookie and save it in err_headers_out
     * so it always gets sent. check_cookie and check_url both will
     * set the N_APP_COOKIE note if they need a new cooke 
     */

    cookie = mwa_remove_note(r, N_APP_COOKIE);
    if (cookie != NULL)
        apr_table_addn(r->err_headers_out, "Set-Cookie", cookie);

    if (subject != NULL) {

        if (in_url && rc.sconf->extra_redirect) {
            return extra_redirect(&rc);
        }
        ap_log_error(APLOG_MARK, APLOG_ERR, 0, r->server,
                     "mod_webauth: check_user_id_hook setting user(%s)",
                     subject);
        r->user = (char*)subject;
        r->ap_auth_type = (char*)at;
        return OK;
    } else {
        if (r->method_number == M_GET) {
            ap_discard_request_body(r);
            return redirect_request_token(&rc);
        } else {
            /* FIXME: redirect to error handler? */
            return HTTP_UNAUTHORIZED;
        }
    }

}

#if 0 
static int 
auth_checker_hook(request_rec *r)
{
    return DECLINED;
}
#endif

/*
 * this hook will attempt to find the returned-token and the
 * state-token in the URL. If we find them and stash them in 
 * the notes for the master request, and then remove them from 
 * everywhere we find them, including the r->the_request, so they 
 * don't show up in access_logs.
 *
 *  we check in the following places:
 *    r->the_request
 *    r->unparsed_uri
 *    r->uri
 *    r->filename
 *    r->canonical_filename
 *    r->path_info
 *    r->args
 *    r->parsed_uri.path
 *    r->parsed_uri.query
 *
 *  we'll stick the token in the notes table for the initial
 *  request
 *  
 */
static int 
translate_name_hook(request_rec *r)
{
    char *p, *s, *rp;
    char *wr, *ws;
    MWA_SCONF *sconf;
    static char *rmagic = WEBAUTHR_MAGIC;
    static char *smagic = WEBAUTHS_MAGIC;


    sconf = (MWA_SCONF*)ap_get_module_config(r->server->module_config,
                                             &webauth_module);
    /*
      ap_log_error(APLOG_MARK, APLOG_ERR, 0, r->server, 
      "mod_webauth: translate_name_hook disabled for now");
      return DECLINED;
    */

    if (!ap_is_initial_req(r)) {
        return DECLINED;
    }

    mwa_log_request(r, "before xlate");

    rp = ap_strstr(r->the_request, rmagic);
    if (rp == NULL) {
        /* no tokens in the request, return */
        return DECLINED;
    }

    /* we need to save the tokens for check_user_id_hook. */

    s = rp+WEBAUTHR_MAGIC_LEN;
    p = ap_strchr(s, ';');
    if (p == NULL) {
        ap_log_error(APLOG_MARK, APLOG_ERR, 0, r->server,
                     "mod_webauth: didn't find end of %s", rmagic);
        return DECLINED;
    }
    wr = apr_pstrmemdup(r->pool, s, p-s);
    ap_log_error(APLOG_MARK, APLOG_ERR, 0, r->server, 
                 "mod_webauth: stash wr(%s)", wr);
    mwa_setn_note(r, N_WEBAUTHR, wr);

    s = p+1;
    p = ap_strstr(s, smagic);
    if (p != NULL) {
        s = p+WEBAUTHS_MAGIC_LEN;
        p = ap_strchr(s, ';');
        if (p == NULL) {
            ap_log_error(APLOG_MARK, APLOG_ERR, 0, r->server,
                         "mod_webauth: didn't find end of %s", smagic);
            return DECLINED;
        }
        ws = apr_pstrmemdup(r->pool, s, p-s);
        ap_log_error(APLOG_MARK, APLOG_ERR, 0, r->server,
                     "mod_webauth: stash ws(%s)", ws);
        mwa_setn_note(r, N_WEBAUTHS, ws);
        s = p+1;
    }

    if (sconf->strip_url) {
        /* move over remaining */
        strcpy(rp, s);
    
        /* these are easier, we strip rmagic and everything after it, 
           which might include smagic */

        strip_end(r->unparsed_uri, rmagic);
        strip_end(r->uri, rmagic);
        strip_end(r->filename, rmagic);
        strip_end(r->canonical_filename, rmagic);
        strip_end(r->path_info, rmagic);
        strip_end(r->args, rmagic);
        strip_end(r->parsed_uri.path, rmagic);
        strip_end(r->parsed_uri.query, rmagic);
    }

    mwa_log_request(r, "after xlate");

    /* still need to return DECLINED, so other modules (like mod_rerewrite)
       get a crack at things */
    return DECLINED;
}

static int 
fixups_hook(request_rec *r)
{
    const char *subject;
    MWA_REQ_CTXT rc;

    rc.r = r;
    rc.dconf = (MWA_DCONF*)ap_get_module_config(r->per_dir_config,
                                                 &webauth_module);

    rc.sconf = (MWA_SCONF*)ap_get_module_config(r->server->module_config,
                                                &webauth_module);

    if (ap_is_initial_req(r)) {
        mwa_log_request(r, "in fixups");
    } else {
        ap_log_error(APLOG_MARK, APLOG_ERR, 0, r->server, 
                     "mod_webauth: subreq fixups url(%s)", r->unparsed_uri);
    }

    /* set environment variable */
    subject = mwa_get_note(r, N_SUBJECT);
    if (subject) {

        char *name;
        if (rc.sconf->var_prefix) {
            name = apr_pstrcat(r->pool, rc.sconf->var_prefix, 
                               ENV_WEBAUTH_USER, NULL);
        } else {
            name = ENV_WEBAUTH_USER;
        }
        apr_table_setn(r->subprocess_env, name, subject);
        /*
        {
            MWA_SERVICE_TOKEN *st = mwa_get_service_token(&rc);
            if (st != NULL) {
                ap_log_error(APLOG_MARK, APLOG_ERR, 0, r->server, 
                             "mod_webauth: st->expires(%d)", st->expires);
            }
        }
        */
    }
    return DECLINED;
}


static int
seconds(const char *value, char **error_str)
{
    char temp[32];
    int mult, len;
    
    len = strlen(value);
    if (len > (sizeof(temp)-1)) {
        *error_str = "error: value too long!";
        return 0;
    }

    strcpy(temp, value);

    switch(temp[len-1]) {
        case 's': 
            mult = 1;
            break;
        case 'm':
            mult = 60;
            break;
        case 'h': 
            mult = 60*60; 
            break;
        case 'd': 
            mult = 60*60*24; 
            break;
        case 'w': 
            mult = 60*60*24*7; 
            break;
        default:
            *error_str = "error: value too long!";
            return 0;
            break;
    }
    
    temp[len-1] = '\0';
    return atoi(temp) * mult;
}

static const char *
cfg_str(cmd_parms *cmd, void *mconf, const char *arg)
{
    int e = (int)cmd->info;
    char *error_str = NULL;
    MWA_DCONF *dconf = (MWA_DCONF *)mconf;

    MWA_SCONF *sconf = (MWA_SCONF *)
        ap_get_module_config(cmd->server->module_config, &webauth_module);
    
    switch (e) {
        /* server configs */
        case E_WebKDCURL:
            sconf->webkdc_url = apr_pstrdup(cmd->pool, arg);
            break;
        case E_WebKDCPrincipal:
            sconf->webkdc_principal = apr_pstrdup(cmd->pool, arg);
            break;
        case E_LoginURL:
            sconf->login_url = apr_pstrdup(cmd->pool, arg);
            break;
        case E_FailureURL:
            sconf->failure_url = apr_pstrdup(cmd->pool, arg);
            break;
        case E_Keyring:
            sconf->keyring_path = ap_server_root_relative(cmd->pool, arg);
            break;
        case E_Keytab:
            sconf->keytab_path = ap_server_root_relative(cmd->pool, arg);
            break;
        case E_ServiceTokenCache:
            sconf->st_cache_path = ap_server_root_relative(cmd->pool, arg);
            break;
        case E_VarPrefix:
            sconf->var_prefix = apr_pstrdup(cmd->pool, arg);
            break;
        case E_SubjectAuthType:
            sconf->subject_auth_type = apr_pstrdup(cmd->pool, arg);
            sconf->subject_auth_type_ex = 1;
            if (strcmp(arg, "krb5") && strcmp(arg,"webkdc")) {
                error_str = apr_psprintf(cmd->pool,
                                         "Invalid value directive %s: %s",
                                         cmd->directive->directive, arg);
            }
            break;
        case E_ReturnURL:
            dconf->return_url = apr_pstrdup(cmd->pool, arg);
            break;
        case E_AppTokenLifetime:
            dconf->app_token_lifetime = seconds(arg, &error_str);
            break;
        case E_TokenMaxTTL:
            sconf->token_max_ttl = seconds(arg, &error_str);
            sconf->token_max_ttl_ex = 1;
            break;
        case E_InactiveExpire:
            dconf->inactive_expire = seconds(arg, &error_str);
            break;
        default:
            error_str = 
                apr_psprintf(cmd->pool,
                             "Invalid value cmd->info(%d) for directive %s",
                             e,
                             cmd->directive->directive);
            break;

    }
    return error_str;
}

static const char *
cfg_flag(cmd_parms *cmd, void *mconfig, int flag)
{
    int e = (int)cmd->info;
    char *error_str = NULL;
    MWA_DCONF *dconf = (MWA_DCONF*) mconfig;

    MWA_SCONF *sconf = (MWA_SCONF *)
        ap_get_module_config(cmd->server->module_config, &webauth_module);
    
    switch (e) {
        /* server configs */
        case E_SecureCookie:
            sconf->secure_cookie = flag;
            sconf->secure_cookie_ex = 1;
            break;
        case E_Debug:
            sconf->debug = flag;
            sconf->debug_ex = 1;
            break;
            /* start of dconfigs */
        case E_ForceLogin:
            dconf->force_login = flag;
            dconf->force_login_ex = 1;
            break;
        case E_StripURL:
            sconf->strip_url = flag;
            sconf->strip_url_ex = 1;
            break;
        case E_ExtraRedirect:
            sconf->extra_redirect = flag;
            sconf->extra_redirect_ex = 1;
            break;
        default:
            error_str = 
                apr_psprintf(cmd->pool,
                             "Invalid value cmd->info(%d) for directive %s",
                             e,
                             cmd->directive->directive);
            break;

    }
    return error_str;
}

#define SSTR(dir,mconfig,help) \
  {dir, (cmd_func)cfg_str,(void*)mconfig, RSRC_CONF, TAKE1, help}

#define SFLAG(dir,mconfig,help) \
  {dir, (cmd_func)cfg_flag,(void*)mconfig, RSRC_CONF, FLAG, help}

#define DSTR(dir,mconfig,help) \
  {dir, (cmd_func)cfg_str,(void*)mconfig, OR_AUTHCFG, TAKE1, help}

#define DFLAG(dir,mconfig,help) \
  {dir, (cmd_func)cfg_flag,(void*)mconfig, OR_AUTHCFG, FLAG, help}

static const command_rec cmds[] = {
    /* server/vhost */
    SSTR(CD_WebKDCURL, E_WebKDCURL, CM_WebKDCURL),
    SSTR(CD_WebKDCPrincipal, E_WebKDCPrincipal, CM_WebKDCPrincipal),
    SSTR(CD_LoginURL, E_LoginURL, CM_LoginURL),
    SSTR(CD_FailureURL, E_FailureURL, CM_FailureURL),
    SSTR(CD_Keyring, E_Keyring, CM_Keyring),
    SSTR(CD_Keytab, E_Keytab,  CM_Keytab),
    SSTR(CD_ServiceTokenCache, E_ServiceTokenCache, CM_ServiceTokenCache),
    SSTR(CD_VarPrefix, E_VarPrefix, CM_VarPrefix),
    SSTR(CD_SubjectAuthType, E_SubjectAuthType, CM_SubjectAuthType),
    SFLAG(CD_StripURL, E_StripURL, CM_StripURL),
    SFLAG(CD_ExtraRedirect, E_ExtraRedirect, CM_ExtraRedirect),
    SFLAG(CD_Debug, E_Debug, CM_Debug),
    SFLAG(CD_SecureCookie, E_SecureCookie, CM_SecureCookie),
    SSTR(CD_TokenMaxTTL, E_TokenMaxTTL, CM_TokenMaxTTL),
    /* directory */
    DSTR(CD_AppTokenLifetime, E_AppTokenLifetime, CM_AppTokenLifetime),
    DSTR(CD_InactiveExpire, E_InactiveExpire, CM_InactiveExpire),
    DFLAG(CD_ForceLogin, E_ForceLogin, CM_ForceLogin),
    DSTR(CD_ReturnURL, E_ReturnURL, CM_ReturnURL),

    { NULL }
};

#undef SSTR
#undef SFLAG
#undef SINT
#undef DSTR
#undef DFLAG
#undef DINT

static void 
register_hooks(apr_pool_t *p)
{
    /* get our module called before the basic authentication stuff */
    static const char * const mods[]={ "mod_access.c", "mod_auth.c", NULL };

    ap_hook_post_config(mod_webauth_init, NULL, NULL, APR_HOOK_MIDDLE);
    ap_hook_child_init(mod_webauth_child_init, NULL, NULL, APR_HOOK_MIDDLE);

    /* we need to get run before anyone else, so we can clean up the URL
       if need be */
    ap_hook_translate_name(translate_name_hook, NULL, NULL, 
                           APR_HOOK_REALLY_FIRST);

    ap_hook_check_user_id(check_user_id_hook, NULL, mods, APR_HOOK_MIDDLE);
    //ap_hook_auth_checker(webauth_auth_checker, NULL, NULL, APR_HOOK_MIDDLE);
    ap_hook_handler(handler_hook, NULL, NULL, APR_HOOK_MIDDLE);
    ap_hook_fixups(fixups_hook, NULL,NULL,APR_HOOK_MIDDLE);
}

/* Dispatch list for API hooks */
module AP_MODULE_DECLARE_DATA webauth_module = {
    STANDARD20_MODULE_STUFF, 
    config_dir_create,     /* create per-dir    config structures */
    config_dir_merge,      /* merge  per-dir    config structures */
    config_server_create,  /* create per-server config structures */
    config_server_merge,   /* merge  per-server config structures */
    cmds,                  /* table of config file commands       */
    register_hooks         /* register hooks                      */
};

