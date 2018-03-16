# Copyright: 2016 Masatake YAMATO
# License: GPL-2

CTAGS="$1 --quiet --options=NONE"

{
{

echo '# NOSUCHLANG'
${CTAGS} --kinddef-NOSUCHLANG=l,name,description --list-kinds-full=NOSUCHLANG

CTAGS="${CTAGS} --langdef=MYTEST"

echo '# define "a"'
${CTAGS} --kinddef-MYTEST=a,aa,aaa --list-kinds-full=MYTEST

echo '# no description 1'
${CTAGS} --kinddef-MYTEST=a,aa, --list-kinds-full=MYTEST

echo '# no description 2'
${CTAGS} --kinddef-MYTEST=a,aa --list-kinds-full=MYTEST

echo '# no name 1'
${CTAGS} --kinddef-MYTEST=a, --list-kinds-full=MYTEST

echo '# no name 2'
${CTAGS} --kinddef-MYTEST=a --list-kinds-full=MYTEST

echo '# an empty name 1'
${CTAGS} --kinddef-MYTEST=a,,x --list-kinds-full=MYTEST

echo '# an empty name 2'
${CTAGS} --kinddef-MYTEST=a,, --list-kinds-full=MYTEST

echo '# wrong letter in name 1'
${CTAGS} --kinddef-MYTEST="a,a	x,d" --list-kinds-full=MYTEST

echo '# wrong letter in name 2'
${CTAGS} --kinddef-MYTEST="a,a	x," --list-kinds-full=MYTEST

echo '# wrong letter in name 3'
${CTAGS} --kinddef-MYTEST="a,a	x" --list-kinds-full=MYTEST

echo '# no letter 1'
${CTAGS} --kinddef-MYTEST=,n,d --list-kinds-full=MYTEST

echo '# no letter 2'
${CTAGS} --kinddef-MYTEST=,n, --list-kinds-full=MYTEST

echo '# no letter 3'
${CTAGS} --kinddef-MYTEST=,n --list-kinds-full=MYTEST

echo '# no letter 4'
${CTAGS} --kinddef-MYTEST=, --list-kinds-full=MYTEST

echo '# no letter 5'
${CTAGS} --kinddef-MYTEST= --list-kinds-full=MYTEST

echo '# wrong letter'
${CTAGS} --kinddef-MYTEST=^ --list-kinds-full=MYTEST

echo '# reusing the letter for file kind'
${CTAGS} --kinddef-MYTEST=F --list-kinds-full=MYTEST

echo '# reusing the name for file kind'
${CTAGS} --kinddef-MYTEST=x,file,desc --list-kinds-full=MYTEST

echo '# inject a flag separator'
${CTAGS} --kinddef-MYTEST='x,kind,desc{foo}' --list-kinds-full=MYTEST

echo '# inject a broken flag separator(1)'
${CTAGS} --kinddef-MYTEST='x,kind,desc{foo' --list-kinds-full=MYTEST

echo '# inject a broken flag separator(2)'
${CTAGS} --kinddef-MYTEST='x,kind,desc{' --list-kinds-full=MYTEST

echo '# use a { in description (1)'
${CTAGS} --kinddef-MYTEST='x,kind,desc\{' --list-kinds-full=MYTEST

echo '# use a { in description (2)'
${CTAGS} --kinddef-MYTEST='x,kind,desc\{}' --list-kinds-full=MYTEST

# echo '# use a { and \t in description'
# ${CTAGS} --kinddef-MYTEST='x,kind,desc\{}\t' --list-kinds-full=MYTEST

echo '# use a \ in description'
${CTAGS} --kinddef-MYTEST='x,kind,desc\\backslash' --list-kinds-full=MYTEST

echo '# description started from {'
${CTAGS} --kinddef-MYTEST='x,kind,{' --list-kinds-full=MYTEST

echo '# description started from \{'
${CTAGS} --kinddef-MYTEST='x,kind,\{' --list-kinds-full=MYTEST

echo '# _refonly flag'
${CTAGS} --kinddef-MYTEST='x,kind,desc' --list-kinds-full=MYTEST
${CTAGS} --kinddef-MYTEST='x,kind,desc' --_roledef-MYTEST=x.role,roleDesc --list-kinds-full=MYTEST
${CTAGS} --kinddef-MYTEST='x,kind,desc{_refonly}' --_roledef-MYTEST=x.role,roleDesc --list-kinds-full=MYTEST

} 2>&1
} | sed -e 's/\.exe//'
