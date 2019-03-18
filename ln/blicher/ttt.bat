

set LN=c:\tools\ln.exe

mkdir s
mkdir s\2
mkdir s\2\3
copy ln.dsp s\2\3
attrib +r s\2\3
attrib +r s\2\3\ln.dsp

mkdir d
mkdir d\2
mkdir d\2\3
copy ln.dsp d\2\3
attrib +r d\2\3
attrib +r d\2\3\ln.dsp

%LN% --splice --mirror s d


