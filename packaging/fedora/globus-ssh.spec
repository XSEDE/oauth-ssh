
%global   _name globus_ssh
%global   soname 0

#
# preamble section
#
Name:     globus-ssh
Summary:  SSH with Globus Auth
Version:  0.1
Release:  1%{?dist}
License:  Proprietary
Group:    Applications/Internet
Source:   %{_name}-%{version}.tar.gz
URL:      https://globus.org/
Vendor:   Globus Support
Packager: Globus Support <support@globus.org>
#%define   debug_package %{nil}

# XXX why do we specify an alternate build root?
BuildRoot:	%{_tmppath}/%{name}-%{version}-%{release}-root-%(%{__id_u} -n)

# XXX we need to figure out how to use cmocka during the build
BuildRequires:  libcurl-devel
BuildRequires:  openssl-devel
BuildRequires:  json-c-devel
BuildRequires:  pam-devel
BuildRequires:  pkgconfig
BuildRequires:  automake
BuildRequires:  libtool
BuildRequires:  libtool-ltdl-devel
BuildRequires:  checkpolicy
BuildRequires:  policycoreutils-python

Requires:  libcurl
Requires:  openssl
Requires:  json-c
Requires:  pam

Requires(post): /usr/sbin/semodule, /usr/sbin/useradd, /usr/bin/getent
Requires(postun): /usr/sbin/userdel, /usr/sbin/semodule

#
# description section
#
%description
SSH with Globus Auth provides a PAM module for use with SSH authentication
in order to introspect OAuth2 access tokens and perform account mapping.

#
# prep section
#
%prep
%setup -q -n %{_name}-%{version}

#
# build section
#
%build

%if %{?fedora}%{!?fedora:0} >= 19 || %{?rhel}%{!?rhel:0} >= 7 || %{?suse_version}%{!?suse_version:0} >= 1315
# Remove files that should be replaced during bootstrap
rm -rf autom4te.cache
autoreconf -if
%endif

%configure \
           --disable-static \
           --docdir=%{_docdir}/%{name}-%{version} \
           --includedir=%{_includedir}/globus \
           --libexecdir=%{_datadir}/globus \
           --libdir=%{_libdir}/security

make %{?_smp_mflags}

#
# check section
#
# XXX not until we can figure out how to add cmocka to the build
# and set LD_LIBRARY_PATH accordingly for 'make check'
#%check
#make check

#
# install section
#
%install
rm -rf $RPM_BUILD_ROOT
make install DESTDIR=$RPM_BUILD_ROOT
rm -f ${RPM_BUILD_ROOT}%{_libdir}/security/pam_globus.la
rm -f ${RPM_BUILD_ROOT}%{_docdir}/%{name}-%{version}/globus-ssh.te

/usr/bin/checkmodule -M -m -o globus-ssh.mod globus-ssh.te
/usr/bin/semodule_package -o ${RPM_BUILD_ROOT}%{_docdir}/%{name}-%{version}/globus-ssh.pp -m globus-ssh.mod

#
# files section
#
%files
%defattr(-,root,root,-)
%{_docdir}/%{name}-%{version}/globus-ssh.pp
%{_libdir}/security/pam_globus.*
%config(noreplace) %{_sysconfdir}/globus/globus-acct-map
%config(noreplace) %attr(0600,root,root) %{_sysconfdir}/globus/globus-ssh.conf

#
# install/uninstall sections
#

%post
/usr/bin/getent passwd globus-mapping || /usr/sbin/useradd -r -s /sbin/nologin globus-mapping
/usr/sbin/semodule -i %{_docdir}/%{name}-%{version}/globus-ssh.pp

%postun
/usr/sbin/userdel globus-mapping
/usr/sbin/semodule -r globus-ssh

#
# clean section
#
%clean
rm -rf $RPM_BUILD_ROOT

#
# changelog section XXX
#
