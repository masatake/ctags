# Copyright: 2021 <YOUR NAME HERE:Please put your ame>
# License: GPL-2

CTAGS=$1

. ../utils.sh

fields=nksSaf

char_expand()
{
	if test -n "$1"; then
		echo ${1:0:1} $(char_expand ${1:1})
	fi
}

s=0
for x in $(char_expand nksSaf); do
	if ${CTAGS} --quiet --options=NONE --fields=+$x --_fatal-warnings /dev/null; then
		echo field $x is available.
	else
		echo field $x is not available.
		s=1
	fi
done

exit $s
