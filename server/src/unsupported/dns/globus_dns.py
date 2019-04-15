import os
import sys
import json
import click
import requests

CTX_CLIENT_ID = 'client_id'
CTX_CLIENT_SECRET = 'client_secret'
CTX_ACCESS_TOKEN = 'access_token'

GLOBUS_AUTH_ENVS = {'preview':     'https://auth.preview.globus.org/v2',
                    'staging':     'https://auth.staging.globuscs.info/v2',
                    'test':        'https://auth.test.globuscs.info/v2',
                    'integration': 'https://auth.integration.globuscs.info/v2',
                    'sandbox':     'https://auth.sandbox.globuscs.info/v2',
                    'production':  'https://auth.globus.org/v2'}

GLOBUS_AUTH_ENDPOINT = GLOBUS_AUTH_ENVS['production']
if os.environ.get('GLOBUS_SDK_ENVIRONMENT') in GLOBUS_AUTH_ENVS:
    GLOBUS_AUTH_ENDPOINT = GLOBUS_AUTH_ENVS[os.environ.get('GLOBUS_SDK_ENVIRONMENT')]

GLOBUS_DNS_ENDPOINT = "https://dns.api.globus.org/v1.0/"
GLOBUS_DNS_SCOPE = "https://auth.globus.org/scopes/glob.us/manage_dns"


@click.group()
@click.option('--client_id', envvar='CLIENT_ID')
@click.option('--client_secret', envvar='CLIENT_SECRET')
@click.pass_context
def entry_point(ctx, client_id, client_secret):
    ctx.obj = {}
    ctx.obj[CTX_CLIENT_ID] = client_id
    ctx.obj[CTX_CLIENT_SECRET] = client_secret

    r = requests.post(GLOBUS_AUTH_ENDPOINT + '/oauth2/token',
                      auth=(client_id, client_secret),
                      data={'scope': GLOBUS_DNS_SCOPE,
                            'grant_type': 'client_credentials'})

    if r.status_code != requests.codes.ok:
        click.echo(r.text)
        sys.exit(1)

    ctx.obj[CTX_ACCESS_TOKEN] = json.loads(r.text)


def build_request_body(a, aaaa, cname):
    body = {}
    if a:
        body['a'] = list(a)
    if aaaa:
        body['aaaa'] = list(aaaa)
    if cname:
        body['cname'] = cname

    if not body:
        click.echo("Specify one of --a, --aaaa or --cname")
        sys.exit(1)

    return body


@click.command()
@click.option('--a', multiple=True)
@click.option('--aaaa', multiple=True)
@click.option('--cname')
@click.pass_context
def create(ctx, a, aaaa, cname):
    body = build_request_body(a, aaaa, cname)
    r = requests.post(GLOBUS_DNS_ENDPOINT + '/domain',
                      headers={'Authorization': 'Bearer ' + ctx.obj[CTX_ACCESS_TOKEN]['access_token']},
                      json=body)

    if r.status_code != requests.codes.ok:
        click.echo(r.text)
        sys.exit(1)

    click.echo(r.json()['domain'])


@click.command()
@click.argument('fqdn')
@click.option('--a', multiple=True)
@click.option('--aaaa', multiple=True)
@click.option('--cname')
@click.pass_context
def update(ctx, fqdn, a, aaaa, cname):
    body = build_request_body(a, aaaa, cname)
    r = requests.put(GLOBUS_DNS_ENDPOINT + '/domain/' + fqdn,
                     headers={'Authorization': 'Bearer ' + ctx.obj[CTX_ACCESS_TOKEN]['access_token']},
                     json=body)

    if r.status_code != requests.codes.ok:
        click.echo(r.text)
        sys.exit(1)

    click.echo("Success")


@click.command()
@click.argument('fqdn')
@click.pass_context
def delete(ctx, fqdn):
    r = requests.delete(GLOBUS_DNS_ENDPOINT + '/domain/' + fqdn,
                        headers={'Authorization': 'Bearer ' + ctx.obj[CTX_ACCESS_TOKEN]['access_token']})

    if r.status_code != requests.codes.ok and r.json()['error_code'] != "NotFound":
        click.echo(r.text)
        sys.exit(1)

    click.echo("Success")


entry_point.add_command(create)
entry_point.add_command(update)
entry_point.add_command(delete)

