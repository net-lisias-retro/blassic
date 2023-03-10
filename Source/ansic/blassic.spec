# RPM .spec file for blassic

# Release number can be specified with rpm --define 'rel SOMETHING' ...
# If no such --define is used, the release number is 1.
#
# Source archive's extension can be specified with rpm --define 'srcext .foo'
# where .foo is the source archive's actual extension.
# To compile an RPM from a .bz2 source archive, give the command
#   rpm -tb --define 'srcext .bz2' blassic-0.7.0.tar.bz2
#
%if %{?rel:0}%{!?rel:1}
%define rel 1
%endif
%if %{?srcext:0}%{!?srcext:1}
%define srcext .gz
%endif

Summary: Classic Basic interpreter
Name: blassic
Version: 0.7.0
Release: %{rel}
Copyright: GPL
Group: Development/Languages
Source: %{name}-%{version}.tar%{srcext}
Prefix: /usr
BuildRoot: %{_tmppath}/%{name}-%{version}-%{release}-root
#Requires: svgalib >= 1.4.0

%description
Blassic is a classic Basic interpreter. The line numbers are
mandatory, and it has PEEK & POKE. The main goal is to execute
programs written in old interpreters, but it can be used as a
scripting language.

%package examples
Summary: Example programs for the Blassic Basic interpreter
Group: Development/Languages

%description examples
Example Basic programs for Blassic, the classic Basic interpreter.


%prep
%setup

%build
./configure --prefix=%{prefix} --enable-installed-examples --disable-svgalib
make CXXFLAGS="-DNDEBUG -O3"

%install
rm -fR $RPM_BUILD_ROOT
make DESTDIR=$RPM_BUILD_ROOT install

%clean
rm -fR $RPM_BUILD_ROOT

%files
%defattr(-, root, root)
%{prefix}/bin/blassic

%files examples
%defattr(-, root, root)
%{prefix}/share/blassic
