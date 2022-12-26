dir=results
mkdir -p $dir
runfile=run.sh
testset=data.test
rm -rf $runfile
for((s=1; s<=10; s++));
do
   for((p=0; p<=1; p++));
   do
      for file in $(cat $testset)
      do
         name=$(basename ${file} .txt)
         echo "./bin/WTA $p 0 $file $s > $dir/${name}_${p}_${s}.out 2>$dir/${name}_${p}_${s}.err" >> $runfile
      done
   done
done

#for((p=8; p<=8; p++));
#do
#   echo "./bin/WTA 1 0 $file > $dir/$file.out 2>$dir/$file.err" > run.sh
#done
   
