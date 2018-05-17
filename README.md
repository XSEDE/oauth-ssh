PAM module for SSH that introspects access tokens passed as passwords during challenge/response.

sudo yum install -y wget cmake gcc
wget https://cmocka.org/files/1.1/cmocka-1.1.1.tar.xz
unxz cmocka-1.1.1.tar.xz
tar xf cmocka-1.1.1.tar
cd cmocka-1.1.1; mkdir build; cd build
cmake ..; make; sudo make install
cd ../..

sudo yum install git autoconf automake libtool curl-devel openssl-devel json-c-devel pam-devel
git clone git@github.com:globusonline/SSH.git
cd SSH
git checkout develop/prototype
autoreconf -i
PKG_CONFIG_PATH=/usr/local/lib/pkgconfig ./configure


\# sufficient causes the prompt to display after the mappings are displayed
\# requisite won't allow us to pass through to another authentication scheme

\# maxtries is returned when account mappings are displayed, so die
\# success is returned when everything is good to go
\# user_unknown is returned when they choose a bad account, so die
\# otherwise, we should fall through to allow others authentication mechs
[maxtries=die success=sufficient default=ignore]

\# Regardless on what we try to do on account unknown, sshd is going to
\# return our pam module. If we want to provide a cleaner ux, we'll need
\# to do it in the client.

auth [pam options] pam_globus.so [globus-ssh.conf [section]]

sudo useradd -s /usr/bin/false -M globus-mapping

/etc/ssh/sshd_config
ChallengeResponseAuthentication yes

/etc/globus/globus-ssh.conf
[default]
auth_client_id  XXXXXXXXXXXXXXXXXXXXXXXXXXXXXX
auth_client_secret  XXXXXXXXXXXXXXXXXXXXXXXXXXXXXX


########################## START SELINUX MODULE globus-ssh.te ##########################

module globus-ssh 1.0;

require {
        type http_port_t;
        type sshd_t;
        class tcp_socket name_connect;
}

\#============= sshd_t ==============

\#!!!! This avc can be allowed using one of the these booleans:
\#     authlogin_yubikey, nis_enabled
allow sshd_t http_port_t:tcp_socket name_connect;

\########################## END SELINUX MODULE globus-ssh.te ##########################

checkmodule -M -m -o globus-ssh.mod globus-ssh.te
semodule_package -o globus-ssh.pp -m globus-ssh.mod
sudo semodule -i globus-ssh.pp
