#! /bin/bash
#
#	Script convertion des fichiers po en fichiers textes 
#	exploitables pour Spacecrafter
#
#	version 0.1
#
#	Association Sirius, Olivier NIVOIX 2020
#	olivier@asso-sirius.org
#


echo $(date)
for file in $(ls *.po)
do
    ./poToMap "$file"
done
echo $(date)
