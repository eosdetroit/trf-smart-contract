#include <eosio/eosio.hpp>
#include "/opt/eosio.contracts/contracts/eosio.token/include/eosio.token/eosio.token.hpp"



// use for determining auto
//template<typename T> struct TD;
// put this after type:
// TD<decltype(unknowntypehere)> td;

class [[eosio::contract("travelrefund")]] travelrefund : public eosio::contract {
    public:
        travelrefund(eosio::name self, eosio::name first_receiver, eosio::datastream<const char *> ds) : eosio::contract(self, first_receiver, ds) {
        }


        [[eosio::action]]
        void create ( eosio::name user) {
            require_auth( user);
            RequestMultiIndex requests( get_self(), get_first_receiver().value );
            print("Creating a new request for ", user);
            RequestIterator iterator = requests.find(user.value);

            //char test = iterator;
			if( iterator == requests.end() ) {
				requests.emplace(user, [&]( RequestStruct & row ) {
					row.user = user;
					row.status 			= "pending";
				});
			} else {
				eosio::print("this user is already in the system with a status of ", iterator->status);
			}
        }

        [[eosio::action]]
        void disburse() {
            require_auth(admin_user);
		  eosio::action transferTest = eosio::action(
			  eosio::permission_level(eosio::name("trfadminuser"),eosio::name("active")),
			  eosio::name("eosio.token"),
			  eosio::name("transfer"),
			  std::make_tuple("trfadminuser", "trfsoulbasis", "0.0001 EOS", "Hope you enjoyed Rio!")
           );
		  transferTest.send();
        }

        [[eosio::action]]
        void process() {
            require_auth(admin_user);
            eosio::asset balance_asset = eosio::token::get_balance(eosio::name("eosio.token"),eosio::name("travelrefund"), eosio::symbol_code("EOS"));
            int balance = balance_asset.amount;
#if 1 
            {
                char balance_str[50];
                sprintf(balance_str,"balance: %d", balance); 

                RequestMultiIndex requests( get_self(), get_first_receiver().value );
                RequestIterator iterator = requests.find(admin_user.value);
                if( iterator == requests.end() ) {
                    requests.emplace(admin_user, [&]( RequestStruct& row ) {
                        row.user = admin_user;
                        row.status 			= balance_str;
                    });
                } else {
                    requests.modify(iterator, admin_user, [&]( RequestStruct& row ) {
                        row.status 			= balance_str;
                    });
                }
            }
#endif


            RequestMultiIndex requests = RequestMultiIndex( get_self(), get_first_receiver().value);
            int sum_distance = 0;
            for (const RequestStruct &item : requests) {
                if (item.status == "approved") {
                    sum_distance += item.distance;
                }
            }
            eosio::print("[sum_distance:    ",  sum_distance);
            PayoutMultiIndex payouts( get_self(), get_first_receiver().value );

            for(PayoutIterator itr = payouts.begin(); itr != payouts.end();) {
                itr = payouts.erase(itr);
            }
            // clear out the payout table
            for (const RequestStruct &item : requests) {
                // erase payout if exists
                if (item.status == "approved") {
                auto payout_found = payouts.find(item.user.value);
                    if (payout_found == payouts.end()) {

                        payouts.emplace(admin_user, [&]( PayoutStruct& row ) {
                            row.user                = item.user;
                            row.status 	            = "processing";
                            row.distance            = item.distance;
                            row.distance_percent    = (float) item.distance / sum_distance;
                            row.amount              = balance * row.distance_percent;
                        });
                    }
                }
            }
        }

        [[eosio::action]]
        void approve(eosio::name user, int distance) {
            require_auth(admin_user);
            if (distance > 0){
                print("Approve request for ", user);
                RequestMultiIndex requests( get_self(), get_first_receiver().value);
                RequestIterator iterator = requests.find(user.value);
                if (iterator != requests.end()) {
                    requests.modify(iterator, admin_user, [&]( RequestStruct& row ) {
                        row.status 			= "approved";
                        row.distance        = distance;
                    });
                } else {
                    printf("\nuser not found\n");
                }
            } else {
                printf("distance must be greater than zero");
            }
        }


        [[eosio::action]]
        void reject ( eosio::name user) {
            require_auth(admin_user);
            print("reject request for ", user);
			RequestMultiIndex requests( get_self(), get_first_receiver().value);
			RequestIterator iterator = requests.find(user.value);
			if (iterator != requests.end()) {
				requests.modify(iterator, admin_user, [&]( RequestStruct& row ) {
					row.status 			= "rejected";
				});
			} else {
				printf("\nuser not found\n");
			}
        }

        [[eosio::action]]
        void erase( eosio::name user) {
            require_auth(admin_user);
            print("erase request for ", user);
			RequestMultiIndex requests( get_self(), get_first_receiver().value);

			RequestIterator iterator = requests.find(user.value);
			if (iterator != requests.end()) {
				requests.erase(iterator);
			} else {
				printf("user not found (already erased?)");
			}
        }

        // @todo(seth) this seems like disburse right?
        [[eosio::action]]
        void disclose( ) {
            require_auth(admin_user);
            printf("Checking to see if any funds are not yet approved or rejected");
			/*
			auto iterator = requests.find(user.value);
			if (iterator != requests.end()) {
				requests.emplace(user, [&]( auto& row ) {
					row.status 			= "approved";
				});
			} else {
				printf("user not found");
			}
			*/
            printf("None found");
            printf("Disclose all funds");
        }
    private:
        eosio::name admin_user = eosio::name("trfadminuser");
        struct [[eosio::table]] RequestStruct{
            eosio::name user;
            std::string status;
            int distance;
            uint64_t primary_key() const { return user.value; }
        };

        typedef eosio::multi_index<eosio::name("requests"), RequestStruct> RequestMultiIndex;
        typedef eosio::multi_index<eosio::name("requests"), RequestStruct>::const_iterator RequestIterator;

        struct [[eosio::table]] PayoutStruct{
            eosio::name user;
            std::string status;
            int distance;
            float distance_percent; 
            float amount;
            uint64_t primary_key() const { return user.value; }
        };
        typedef eosio::multi_index<eosio::name("payouts"), PayoutStruct> PayoutMultiIndex;
        typedef eosio::multi_index<eosio::name("payouts"), PayoutStruct>::const_iterator PayoutIterator;
        
};

