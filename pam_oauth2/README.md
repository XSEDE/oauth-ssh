OAuth2 PAM module
=================

This PAM module enables login with OAuth2 token instead of password.

## How to install it:

```bash
$ sudo apt-get install libcurl4-openssl-dev libpam0g-dev
$ git submodule init
$ git submodule update
$ make
$ sudo make install
```

## Configuration

```
auth sufficient pam_oauth2.so <introspect url> <clientid>:<client secret> <username field> key1=value2 key2=value
account sufficient pam_oauth2.so
```

## How it works

Lets assume that configuration is looking like:

```
auth sufficient pam_oauth2.so https://auth.globus.org/v2/oauth2/token/introspect username scope=urn:globus:auth:scope:trooper.mikelink.com:ssh_login 
```

And somebody is trying to login with login=foo and token=bar.

pam\_oauth2 module will make http post request for introspection to url with token=bar as post data, and check response code and content.

If the response code is not 200 - authentication will fail. After that it will check response content:

```json
{
  "access_token": "bar",
  "expires_in": 3598,
  "scope": "urn:globus:auth:scope:trooper.mikelink.com:ssh_login",
  "username": "foo"
}
```

It will check that response is a valid JSON object and top-level object contains following key-value pairs:
```json
  "scope": "urn:globus:auth:scope:trooper.mikelink.com:ssh_login",
  "username": "foo"
```

If some keys haven't been found or values don't match with expectation - authentication will fail.

### Issues and Contributing

Oauth2 PAM module welcomes questions via our [issues tracker](https://github.com/zalando-incubator/pam-oauth2/issues). We also greatly appreciate fixes, feature requests, and updates; before submitting a pull request, please visit our [contributor guidelines](https://github.com/zalando-incubator/pam-oauth2/blob/master/CONTRIBUTING.rst).

License
-------

This project uses the [MIT license](https://github.com/zalando-incubator/pam-oauth2/blob/master/LICENSE). 
