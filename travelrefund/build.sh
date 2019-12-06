
set -e 


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
} 

start=0
unlock=0
build=0
run_test=0


if [ $# -eq 0 ]; then
	print_help
    read -p "List Options: (-n -u -b) " -r $REPLY
    set -- $REPLY
fi

while getopts ":nubt" opt; do
  case ${opt} in
	n ) start=1 ;;
	u ) unlock=1 ;;
	b ) build=1 ;;
	t ) run_test=1 ;;
	\? ) print_help; exit 1 ;;
  esac
done

START=$(date +%s);

if [ $run_test = 1 ]; then
	print_heading "running test"
    gcc -o exec_test test.cpp
    ./exec_test
    print_time_passed
    exit
fi

if [ $start = 1 ]; then
	print_heading "starting node"
    nodeos -e -p eosio \
        --plugin eosio::producer_plugin \
        --plugin eosio::chain_api_plugin \
        --plugin eosio::http_plugin \
        --plugin eosio::history_plugin \
        --plugin eosio::history_api_plugin \
        --filter-on="*" \
        --access-control-allow-origin='*' \
        --contracts-console \
        --http-validate-host=false \
        --verbose-http-errors >> /dev/null 2>&1 &
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
#domain="http://localhost:8888"
ADMIN_USERNAME="trfadminuser"
if [ 0 = 1 ]; then
    cleos -u $domain set account permission travelrefund active --add-code
fi


if [ 1 = 1 ]; then
    print_heading "set contract"
	cleos -u $domain set contract travelrefund . -p travelrefund@active
fi

if [ 1 = 1 ]; then
    print_heading "erase"
	cleos -u $domain push action travelrefund erase '["trfadminuser"]' -p trfadminuser@active
	cleos -u $domain push action travelrefund erase '["trfsourbasis"]' -p trfadminuser@active
	cleos -u $domain push action travelrefund erase '["trftidyfairy"]' -p trfadminuser@active
fi
if [ 1 = 1 ]; then
    print_heading "create"
	cleos -u $domain push action travelrefund create '["trfadminuser"]' -p trfadminuser@active
	cleos -u $domain push action travelrefund create '["trftidyfairy"]' -p trftidyfairy@active
	cleos -u $domain push action travelrefund create '["trfsourbasis"]' -p trfsourbasis@active
	cleos -u $domain push action travelrefund approve '["trftidyfairy", 60]' -p trfadminuser@active
	cleos -u $domain push action travelrefund approve '["trfsourbasis", 40]' -p trfadminuser@active
	#cleos -u $domain push action travelrefund reject '["trftidyfairy"]' -p trfadminuser@active
	#cleos push action travelrefund disclose '[]' -p trfsourbasis@active
fi
if [ 1 = 1 ]; then
    print_heading "process"
	cleos -u $domain push action travelrefund process '[]' -p trfadminuser@active
fi

if [ 0 = 1 ]; then
    print_heading "disburse"
	cleos -u $domain push action travelrefund disburse '[]' -p trfadminuser@active
fi

if [ 1 = 1 ]; then
    print_heading "get table"
    cleos -u $domain get table travelrefund travelrefund requests 
    cleos -u $domain get table travelrefund travelrefund payouts 
fi

print_time_passed

