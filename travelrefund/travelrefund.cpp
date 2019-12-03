#include <eosio/eosio.hpp>
#include "/opt/eosio.contracts/contracts/eosio.token/include/eosio.token/eosio.token.hpp"




class [[eosio::contract("travelrefund")]] travelrefund : public eosio::contract {
    public:
        travelrefund(eosio::name self, eosio::name first_receiver, eosio::datastream<const char *> ds) : eosio::contract(self, first_receiver, ds) {
        }


        [[eosio::action]]
        void create ( eosio::name user) {
            require_auth( user);
            request_multi_index requests( get_self(), get_first_receiver().value );
            print("Creating a new request for ", user);
            auto iterator = requests.find(user.value);
			if( iterator == requests.end() ) {
				requests.emplace(user, [&]( auto& row ) {
					row.user = user;
					row.status 			= "pending";
				});
			} else {
				eosio::print("this user is already in the system with a status of ", iterator->status);
			}
        }

        [[eosio::action]]
        void process() {
            require_auth(admin_user);
            // get balance
            // auto balance = eosio::token::get_balance(token_contract, token_holder_name, symbol_code);
            //
            /*
         static asset get_balance( const name& token_contract_account, const name& owner, const symbol_code& sym_code )
         {
            accounts accountstable( token_contract_account, owner.value );
            const auto& ac = accountstable.get( sym_code.raw() );
            return ac.balance;
         }
/orgs/eos_detroit/42_trf_smart_contract/travelrefund/travelrefund.cpp:43:54: error: reference to type 'const eosio::name' could not bind to an lvalue of type 'const char [13]'
            auto balance = eosio::token::get_balance("travelrefund", admin_user, "eosio.token");
                                                     ^~~~~~~~~~~~~~
                                               ^
         */
            const auto sym_code= eosio::symbol_code("EOS");
            // assertion failure with message: only uppercase letters allowed in symbol_code string
            //
            eosio::asset balance_asset = eosio::token::get_balance(eosio::name("travelrefund"), admin_user, sym_code);
            //float balance = balance_asset.amount;
            float balance = 0;

            /*
        eosio::token t(N(eosio.token));
        const auto sym_name = eosio::symbol_type(S(4,SYS)).name();
        const auto my_balance = t.get_balance(N(myaccount), sym_name );
        eosio::print("My balance is ", my_balance);
        */

            request_multi_index requests = request_multi_index( get_self(), get_first_receiver().value);
            float sum_distance = 0;
            for (auto &item : requests) {
                if (item.status == "approved") {
                    sum_distance += item.distance;
                }
            }
            eosio::print("[sum_distance:    ",  sum_distance);
            payout_multi_index payouts( get_self(), get_first_receiver().value );
            for(auto itr = payouts.begin(); itr != payouts.end();) {
                itr = payouts.erase(itr);
            }
            // clear out the payout table
            for (auto &item : requests) {
                // erase payout if exists
                if (item.status == "approved") {
                auto payout_found = payouts.find(item.user.value);
                    if (payout_found == payouts.end()) {
                        payouts.emplace(admin_user, [&]( auto& row ) {
                            row.user                = item.user;
                            row.status 	            = "processing";
                            row.distance            = item.distance;
                            row.distance_percent    = item.distance/sum_distance;
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
                request_multi_index requests( get_self(), get_first_receiver().value);
                auto iterator = requests.find(user.value);
                if (iterator != requests.end()) {
                    requests.modify(iterator, admin_user, [&]( auto& row ) {
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
			request_multi_index requests( get_self(), get_first_receiver().value);
			auto iterator = requests.find(user.value);
			if (iterator != requests.end()) {
				requests.modify(iterator, admin_user, [&]( auto& row ) {
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
			request_multi_index requests( get_self(), get_first_receiver().value);

			auto iterator = requests.find(user.value);
			if (iterator != requests.end()) {
				requests.erase(iterator);
			} else {
				printf("user not found (already erased?)");
			}
        }

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
        struct [[eosio::table]] request_struct{
            eosio::name user;
            std::string status;
            int distance;
            uint64_t primary_key() const { return user.value; }
        };
        typedef eosio::multi_index<eosio::name("requests"), request_struct> request_multi_index;

        struct [[eosio::table]] payout_struct{
            eosio::name user;
            std::string status;
            float distance;
            float distance_percent; 
            float amount;
            uint64_t primary_key() const { return user.value; }
        };
        typedef eosio::multi_index<eosio::name("payouts"), payout_struct> payout_multi_index;
        
};

