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
	eosio-cpp  -o trf.o  -abigen trf.cpp
	time_passed
fi

if [ 1 = 1 ]; then
	echo "full build"
	#eosio-ld -o trf.wasm trf.o
	eosio-cpp -abigen -o trf.wasm trf.cpp
	echo "done"
	time_passed
fi

if [ 1 = 1 ]; then
	cleos wallet unlock
	ADMIN_USERNAME="bvizeqdoq5ga"
    echo "set contract"
	cleos set contract trf . -p trf@active
    echo "push"
	cleos push action trf erase '[""]' -p bob@active
	cleos push action trf erase '["bob"]' -p bob@active
	cleos push action trf erase '["alice"]' -p bob@active
	cleos push action trf create '["alice"]' -p alice@active
	cleos push action trf create '["bob"]' -p bob@active
	cleos push action trf approve '["bob", 10]' -p bob@active
	#cleos push action trf approve '["bob", 10]' -p bob@active
	cleos push action trf reject '["alice"]' -p bob@active
	#cleos push action trf disclose '[]' -p bob@active
fi

if [ 1 = 1 ]; then
    cleos get table trf trf requests 
fi
