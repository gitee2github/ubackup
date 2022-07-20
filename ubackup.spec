Name:           com.deepin.backup
Version:        1.0
Release:        0
Source:         %{name}-%{version}.tar.gz

BuildRequires:  boost-devel
BuildRequires:  libtool
BuildRequires:  gcc-c++

Requires: restic

Summary:        UOS server backup tool
License:        GPLv2

%description
This package contains ubackup, a tool for UOS server backup.

%package grub2
Summary:       A service for ubackup tools,when os starup in initramfs
BuildRequires: grub2-tools systemd
License:       GPLv2
Provides:      ubackup-grub2

%description grub2
A service for ubackup tools,when os starup in initramfs

%package dracut
Summary:       Dracut module to build an initramfs with ubackup
License:       GPLv2
Provides:      ubackup-dracut
Requires:      %{name} = %{version}-%{release}

%description dracut
This package provides a dracut module to build an initramfs

%prep
%setup

%build
touch README
aclocal
libtoolize --force --automake --copy
autoheader
automake --add-missing --copy
autoconf

%configure --libdir=%{_libdir}
make

%install
%make_install
install -D -m 0644 ubackup.json "%{buildroot}/etc/ubackup/ubackup.json"

install -d -m 0755 %{buildroot}%{_bindir}/
install -d -m 0755 %{buildroot}%{_sysconfdir}/ubackup/profile.d/
install -d -m 0755 %{buildroot}%{_prefix}/lib/dracut/modules.d/99ubackup/
install -d -m 0755 %{buildroot}%{_sysconfdir}/grub.d/  
install -d -m 0755 %{buildroot}%{_sysconfdir}/ubackup/service/
install -d -m 0755 %{buildroot}%{_sysconfdir}/ubackup/grub2/

install -p -m 0755 grub2/ubackup_restore %{buildroot}%{_bindir}/
install -p -m 0644 grub2/ubackuplib.sh %{buildroot}%{_sysconfdir}/ubackup/profile.d/
install -p -m 0755 grub2/urestore.sh %{buildroot}%{_sysconfdir}/ubackup/profile.d/
install -p -m 0644 dracut-module/module-setup.sh %{buildroot}%{_prefix}/lib/dracut/modules.d/99ubackup/
install -p -m 0755 grub2/50_ubackup %{buildroot}%{_sysconfdir}/grub.d/  
install -p -m 0644 service/ubackup-restore.service %{buildroot}%{_sysconfdir}/ubackup/service/

%check

%files
%defattr(-,root,root)
%license LICENSE
%{_sysconfdir}/ubackup/profile.d/ubackuplib.sh
%{_bindir}/ubackup
%{_libdir}/libubackup.so
%{_includedir}/ubackup.h
%{_libdir}/libubackup.so.*
%{_libdir}/libubackup.a
%{_libdir}/libubackup.la
/etc/ubackup/ubackup.json

%files grub2
%defattr(-,root,root)
%license LICENSE
%{_bindir}/ubackup_restore
%{_sysconfdir}/grub.d/50_ubackup
%{_sysconfdir}/ubackup/profile.d/ubackuplib.sh
%{_sysconfdir}/ubackup/profile.d/urestore.sh
%{_sysconfdir}/ubackup/service/ubackup-restore.service
%dir %{_sysconfdir}/ubackup/grub2/

%files dracut
%defattr(-,root,root)
%license LICENSE
%{_prefix}/lib/dracut/modules.d/99ubackup/module-setup.sh

%changelog
* Tue Aug 31 2021 heyitao <heyitao@uniontech.com> - 1.0-0
- Add grub2 function for backup-restore tools

