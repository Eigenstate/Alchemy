#!/bin/bash
#
#    Alchemy, a metabolic pathway generator 
#    Copyright (C) 2013  Robin Betz
# 
#    This program is free software: you can redistribute it and/or modify
#    it under the terms of the GNU General Public License as published by
#    the Free Software Foundation, either version 3 of the License, or
#    (at your option) any later version.
#
#    This program is distributed in the hope that it will be useful,
#    but WITHOUT ANY WARRANTY; without even the implied warranty of
#    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#    GNU General Public License for more details.
#
#    You should have received a copy of the GNU General Public License
#    along with this program.  If not, see <http://www.gnu.org/licenses/>.
#

ALCHEMYDIR="/home/robin/Schoolwork/2012-2013/BIMM185/Project/cpp"
mode=$(kdialog --title "Pathfinding Criterion" --radiolist "Select pathfinding criterion:" 1 "Fewest molecules" on 2 "Fewest enzymes" off)
sub=$(kdialog --title "Substrate" --inputbox "Enter starting material" "")
slist=$(mysql -B -u robin -ptestpw -e "SELECT kegg_id FROM molecules WHERE name='$sub'" alchemy | tail -n 1)
while [ -z "$slist" ]; do
  sub=$(kdialog --title "Substrate" --inputbox "NO SUCH MOLECULE!\nEnter starting material" "")
  slist=$(mysql -B -u robin -ptestpw -e "SELECT kegg_id FROM molecules WHERE name='$sub'" alchemy | tail -n 1)
done

prod=$(kdialog --title "Product" --inputbox "Enter desired final material" "")
plist=$(mysql -B -u robin -ptestpw -e "SELECT kegg_id FROM molecules WHERE name='$prod'" alchemy | tail -n 1)
while [ -z "$plist" ]; do
  prod=$(kdialog --title "Product" --inputbox "NO SUCH MOLECULE!\nEnter desired final material" "")
  plist=$(mysql -B -u robin -ptestpw -e "SELECT kegg_id FROM molecules WHERE name='$prod'" alchemy | tail -n 1)
done

command="./alchemy $mode \"$slist\" \"$plist\""
echo $command
$ALCHEMYDIR/alchemy $mode "$slist" "$plist"
if [ $? == 0 ]; then
  display output.png
else
  kdialog --sorry "No path found"
fi
