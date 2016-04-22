#!/bin/bash

if [ -z "$1" ] || ! [ "$1" -eq "$1" 2>/dev/null ]; then
        echo "usage: `basename $0` <n>"
        echo "n - the number of patches need to be checked on top of HEAD"
        exit 1
fi

num_patches=${1#-}
failed=0
for (( i = $num_patches - 1; i >= 0; i-- )); do
        patch=`git log -1 HEAD~$i --pretty=format:'%h("%s")%n'`
        echo -e "\033[32mchecking patch $patch...\033[0m"
        git format-patch --stdout -1 HEAD~$i | scripts/checkpatch.pl -
	if (($? > 0)); then
		((failed++))
	fi
        echo ""
done

if (($failed > 0)); then
	echo -e "\033[31mResult: total $failed/$num_patches patches have style problem, please fix them.\033[0m"
else
	echo -e "\033[32mResult: all $num_patches patches are good.\033[0m"
fi
