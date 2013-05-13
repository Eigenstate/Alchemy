#!/bin/bash

mode=$(kdialog --radiolist "Select pathfinding criterion:" 1 "Fewest molecules" on 2 "Fewest enzymes" off)
sub=$(kdialog --title "Substrate" --inputbox "Enter starting material" "")
slist=$(mysql -B -u robin -ptestpw -e "select idx from molecules where name='$sub'" alchemy | tail -n 1)
while [ -z "$slist" ]; do
  sub=$(kdialog --title "Substrate" --inputbox "NO SUCH MOLECULE!\nEnter starting material" "")
  slist=$(mysql -B -u robin -ptestpw -e "select idx from molecules where name='$sub'" alchemy | tail -n 1)
done

prod=$(kdialog --title "Product" --inputbox "Enter desired final material" "")
plist=$(mysql -B -u robin -ptestpw -e "select idx from molecules where name='$prod'" alchemy | tail -n 1)
while [ -z "$plist" ]; do
  prod=$(kdialog --title "Product" --inputbox "NO SUCH MOLECULE!\nEnter desired final material" "")
  plist=$(mysql -B -u robin -ptestpw -e "select idx from molecules where name='$prod'" alchemy | tail -n 1)
done

command="./alchemy $mode $slist $plist"
echo $command
./cpp/alchemy $mode $slist $plist
if [ $? == 0 ]; then
  display output.png
else
  kdialog --sorry "No path found"
fi
