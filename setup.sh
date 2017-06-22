DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
export LD_LIBRARY_PATH=$DIR/lib:$LD_LIBRARY_PATH
export JETNET_PATH=$DIR
export PATH=$JETNET_PATH/bin:$PATH
echo $JETNET_PATH

