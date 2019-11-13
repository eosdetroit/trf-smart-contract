set -e 
clear && printf '\e[3J'
START=$(date +%s);

time_passed () {
	DT=$[$(date +%s)-$START];
	SECS=$[$DT % 60];
	TOTAL_MINS=$[$DT / 60];
	MINS=$[TOTAL_MINS % 60];
	HOURS=$[TOTAL_MINS / 60];
	printf "%02d:%02d:%02d\n" $HOURS $MINS $SECS
}

date

build=1
unlock=${1:-0}


if [ $build = 1 ]; then
    echo "[build]"
	eosio-cpp -abigen -o travelrefund.wasm travelrefund.cpp
    echo "[done]"
	time_passed
fi

if [ $unlock = 1 ]; then
	cleos wallet unlock 
fi

#domain="https://api.jungle.alohaeos.com"
domain="http://localhost:8888"
ADMIN_USERNAME="trfadminuser"

if [ 1 = 1 ]; then
    echo "[set contract]"
	cleos -u $domain set contract travelrefund . -p travelrefund@active
    echo "[done]"
fi

if [ 0 = 1 ]; then
	cleos -u $domain push action travelrefund erase '["trfsourbasis"]' -p trfadminuser@active
	cleos -u $domain push action travelrefund erase '["trftidyfairy"]' -p trfadminuser@active
fi
if [ 1 = 1 ]; then
    echo "[process]"
	cleos -u $domain push action travelrefund process '[]' -p trfadminuser@active
    echo "[done]"
fi
if [ 1 = 0 ]; then
    echo "push"
	cleos -u $domain push action travelrefund create '["trfadminuser"]' -p trfadminuser@active
	cleos -u $domain push action travelrefund create '["trftidyfairy"]' -p trftidyfairy@active
	cleos -u $domain push action travelrefund create '["trfsourbasis"]' -p trfsourbasis@active
	cleos -u $domain push action travelrefund approve '["trfadminuser", 120]' -p trfadminuser@active
	cleos -u $domain push action travelrefund approve '["trftidyfairy", 60]' -p trfadminuser@active
	cleos -u $domain push action travelrefund approve '["trfsourbasis", 10]' -p trfadminuser@active
	#cleos push action travelrefund approve '["trfsourbasis", 10]' -p trfsourbasis@active
	cleos -u $domain push action travelrefund reject '["trftidyfairy"]' -p trfadminuser@active
	#cleos push action travelrefund disclose '[]' -p trfsourbasis@active
fi

if [ 1 = 1 ]; then
    echo "get table"
    cleos -u $domain get table travelrefund travelrefund requests 
    cleos -u $domain get table travelrefund travelrefund payouts 
fi
