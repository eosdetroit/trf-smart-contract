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

if [ 0 = 1 ]; then
	echo "preprocess, compile, assemble"
	eosio-cpp  -o travelrefund.o  -abigen travelrefund.cpp
	time_passed
fi

if [ 1 = 1 ]; then
	echo "full build"
	#eosio-ld -o travelrefund.wasm travelrefund.o
	eosio-cpp -abigen -o travelrefund.wasm travelrefund.cpp
	echo "done"
	time_passed
fi

if [ 1 = 1 ]; then
	#cleos wallet unlock
	ADMIN_USERNAME="bvizeqdoq5ga"
    echo "set contract"
	cleos -u https://api.jungle.alohaeos.com set contract travelrefund . -p wigglewiggle@active
    echo "push"
	cleos push action travelrefund erase '[""]' -p bob@active
	cleos push action travelrefund erase '["bob"]' -p bob@active
	cleos push action travelrefund erase '["alice"]' -p bob@active
	cleos push action travelrefund create '["alice"]' -p alice@active
	cleos push action travelrefund create '["bob"]' -p bob@active
	cleos push action travelrefund approve '["bob", 10]' -p bob@active
	#cleos push action travelrefund approve '["bob", 10]' -p bob@active
	cleos push action travelrefund reject '["alice"]' -p bob@active
	#cleos push action travelrefund disclose '[]' -p bob@active
fi

if [ 1 = 1 ]; then
    cleos get table travelrefund travelrefund requests 
fi
