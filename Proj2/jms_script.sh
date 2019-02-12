#!/bin/bash

flag1=0

if [ $# = 5 ]
then

flag1=1

for ((i=0;i<=$#;i++)); 
do 
if [ ${!i} = -c ]
then
flag2="$i"
fi
done

if [ $flag2 = 1 ]
then
SIZE="$3"
fi

if [ $flag2 = 3 ]
then
SIZE="$5"
fi


fi



while [[ $# -gt 1 ]]
do
key="$1"

case $key in
    -l|--path)
    MYPATH="$2"
    shift # past argument
    ;;
    -c|--command)
    COMMAND="$2"
    shift # past argument
    ;;
    --default)
    DEFAULT=YES
    ;;
    *)
            # unknown option
    ;;
esac
shift # past argument or value
done

echo NUM = "${SIZE}"
echo MYPATH  = "${MYPATH}"
echo COMMAND     = "${COMMAND}"

DIRS=`ls $MYPATH`

if [ ${COMMAND} = list ]
then
for DIR in $DIRS
do
echo  ${DIR}
done
fi

if [ ${COMMAND} = size ]
then

if [ $flag1 = 0 ]
then  
du -h $MYPATH | sort -h
fi

if [ $flag1 = 1 ]
then  
du -h $MYPATH | sort -h | tail -n ${SIZE}
fi

fi

if [ ${COMMAND} = purge ]
then
rm -r "${MYPATH}"
fi







