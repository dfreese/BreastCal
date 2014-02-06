#source /home/miil/MODULE_ANA/ANA_V5/ModuleClass/src/addtopath.sh /home/miil/MODULE_ANA/ANA_V5/ModuleClass
# Got this from stack overflow
DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"

echo $DIR
source $DIR/src/addtopath.sh $DIR