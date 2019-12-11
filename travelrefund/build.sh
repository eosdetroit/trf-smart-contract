
myfunc() {
	print_time_passed
	exit
}

trap 'myfunc' ERR



clear && printf '\e[3J'




print_heading () {
	printf "\n\n******************** $1 ************************\n\n" 
}
print_time_passed () {
	DT=$[$(date +%s)-$START];
	SECS=$[$DT % 60];
	TOTAL_MINS=$[$DT / 60];
	MINS=$[TOTAL_MINS % 60];
	HOURS=$[TOTAL_MINS / 60];
	printf -v formatted_time "Time Passed: %02d:%02d:%02d" $HOURS $MINS $SECS
    print_heading "$formatted_time"
}
function print_help () {
	echo "./build.sh [-n] [-u] [-b]"
	echo "-n start up node"
	echo "-u unlock wallet"
	echo "-b build"
	echo "-t test"
	echo "-a run through all"
} 

start=0
unlock=0
build=0
run_test=0

change_permission=0
erase=1
create=1
approve=1
process=1
disburse=1
complete=1

if [ $# -eq 0 ]; then
	print_help
    read -p "List Options: (-n -u -b) " -r $REPLY
    set -- $REPLY
fi

while getopts ":nubta" opt; do
  case ${opt} in
	n ) start=1 ;;
	u ) unlock=1 ;;
	b ) build=1 ;;
	t ) run_test=1 ;;
    a ) 
        build=1
        erase=1
        create=1
        approve=1
        process=1
        disburse=1;;
	\? ) print_help; exit 1 ;;
  esac
done

START=$(date +%s);

if [ $run_test = 1 ]; then
	print_heading "running test"
    ./test.cpp
    print_time_passed
    exit
fi

if [ $start = 1 ]; then
	print_heading "starting node"
    nodeos -e -p eosio \
        --delete-all-blocks \
        --hard-replay \
        --plugin eosio::producer_plugin \
        --plugin eosio::chain_api_plugin \
        --plugin eosio::http_plugin \
        --plugin eosio::history_plugin \
        --plugin eosio::history_api_plugin \
        --filter-on="*" \
        --access-control-allow-origin='*' \
        --contracts-console \
        --http-validate-host=false \
        --verbose-http-errors 
    exit
fi

if [ $build = 1 ]; then
    print_heading "build"
	#eosio-cpp -DCORE_SYMBOL_NAME=EOS -abigen -o travelrefund.wasm travelrefund.cpp
	eosio-cpp -abigen -o travelrefund.wasm travelrefund.cpp
	print_time_passed
fi


if [ $unlock = 1 ]; then
	print_heading "Unlock cleos wallet"
	cleos wallet unlock 
fi


domain="https://api.jungle.alohaeos.com"
domain="https://jungle2.cryptolions.io"
#domain="http://localhost:8888"
ADMIN_USERNAME="trfadminuser"

if [ $change_permission = 1 ]; then
    cleos -u $domain set account permission travelrefund active --add-code
fi

if [ $build = 1 ]; then
    print_heading "set contract"
	cleos -u $domain set contract travelrefund . -p travelrefund@active
fi


user_accounts=("trftidyfairy" "trfsourbasis" "wittyurgency" "vaininventor" "offbeatgauge" "niftysuccess" "richmovement" "shabbyintent" "smoggyoutlet" "smoothschool" "spicycharity" "tenderwinter" )
user_count=12


# Create random distances
RANDOM=1 # seed
distances=()
print_heading "test users"
for (( i=0; i < $user_count; ++i )); do
    distances+=( $RANDOM )
    printf "${user_accounts[i]} \t ${distances[i]}\n"
done

if [ $erase = 1 ]; then
    print_heading "erase"
    for user in ${user_accounts[@]}; do 
        echo
        cleos -u $domain push action travelrefund erase "[\"${user}\"]" -p trfadminuser@active
    done
fi


if [ $create = 1 ]; then
    print_heading "create"
    for user in ${user_accounts[@]}; do 
        echo
        cleos -u $domain push action travelrefund create "[\"${user}\"]" -p ${user}@active
    done
fi
if [ $approve = 1 ]; then
    print_heading "approve"
    for idx in ${!user_accounts[@]}; do 
        username=${user_accounts[idx]}
        distance=${distances[idx]}
        echo
        cleos -u $domain push action travelrefund approve "[\"${username}\", ${distance}], " -p trfadminuser@active
    done
fi
if [ $process = 1 ]; then
    print_heading "process"
	cleos -u $domain push action travelrefund process '[]' -p trfadminuser@active
fi

if [ $disburse = 1 ]; then
    print_heading "disburse"
	cleos -u $domain push action travelrefund disburse '[]' -p travelrefund@active
fi

if [ $complete = 1 ]; then
    print_heading "complete"
    for user in ${user_accounts[@]}; do 
        echo
        cleos -u $domain push action travelrefund complete "[\"${user}\"]" -p trfadminuser@active
    done
fi

if [ 1 = 1 ]; then
    print_heading "get table"
    cleos -u $domain get table travelrefund travelrefund requests 
    cleos -u $domain get table travelrefund travelrefund payouts 
fi

print_time_passed

