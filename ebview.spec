Summary: EPWING CD-ROM dictionary viewer
Name: ebview
Version: 0.3.6
Release: 1
Copyright: GPL
Group: Applications/Text
Source: http://prdownloads.sourceforge.net/ebview/ebview-%{version}.tar.gz
URL: http://ebview.sourceforge.net/
Prefix: /usr
BuildRoot: %{_tmppath}/%{name}-%{version}-root
Summary(ja): EPWING形式のCD-ROM辞書を参照するためのプログラム
%description
An EPWING CD-ROM dictionary viewer.
Requeires: gtk2 >= 2.2, eb >= 3.3.2

%description -l ja
EPWING形式のCD-ROM辞書を参照するためのプログラムで、次の特徴があります。
    * 前方一致、後方一致、完全一致、条件一致、複合検索、これらを組み合わせたおまかせ検索など、さまざまな検索方法が用意されています。
    * 串刺し検索:複数の辞書を一気に検索します。
    * 外字、静止画、動画、音声の表示や再生ができます。
    * Xセレクションの自動的な検索が可能です。たとえば、Mozillaで英文ページを読んでいる場合に、分からない単語があったらその単語を選択することで自動的に検索されます。
    * 語尾の自動補正を行いますので、英単語の過去形や複数形、日本語の活用などが補正された形で検索されます。

%prep

%setup -q -c
cd %{name}-%{version}
#%patch1 -p1
cd ..

%build
cd %{name}-%{version}
autoconf
%configure --with-eb-conf=/etc/eb.conf
make
cd ..

%install
cd %{name}-%{version}
%makeinstall
cd ..

%clean
rm -rf ${RPM_BUILD_ROOT}
rm -f *.files

%files
/usr/bin/ebview
/usr/share/locale/ja/LC_MESSAGES/ebview.mo
/usr/share/ebview/about.jp
/usr/share/ebview/about.en
/usr/share/ebview/endinglist.xml
/usr/share/ebview/endinglist-ja.xml
/usr/share/ebview/shortcut.xml
/usr/share/ebview/searchengines.xml
/usr/share/ebview/filter.xml
/usr/share/ebview/help/ja/index.html
/usr/share/ebview/help/ja/menu.html
/usr/share/ebview/help/ja/body.html
/usr/share/ebview/help/en/index.html
/usr/share/ebview/help/en/menu.html
/usr/share/ebview/help/en/body.html
%defattr(-, root, root)
%doc %{name}-%{version}/ChangeLog
%doc %{name}-%{version}/README

%changelog
* Thu May 22 2003 Kenichi Suto <deep_blue@users.sourceforge.net>
- version 0.3.0
* Tue Nov 19 2002 Kenichi Suto <deep_blue@users.sourceforge.net>
- version 0.2.0
* Fri May 17 2002 Kenichi Suto <deep_blue@users.sourceforge.net>
- version 0.1.5
* Sun Feb 24 2002 Kenichi Suto <deep_blue@users.sourceforge.net>
- version 0.1.4
* Fri Jul 27 2001 Kenichi Suto <deep_blue@users.sourceforge.net>
- version 0.1.2
* Fri Jun 22 2001 akira yamada <akira@vinelinux.org>
- Initial packaging.
