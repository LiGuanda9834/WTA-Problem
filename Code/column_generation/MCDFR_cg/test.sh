demand=demandfile
edge=edgefile
datadir=../data

exe=runMCFDR
commands="./$exe ${datadir}/${demand} ${datadir}/${edge}"

make -j
./$exe ${datadir}/${demand} ${datadir}/${edge}

echo "./val $commands"
