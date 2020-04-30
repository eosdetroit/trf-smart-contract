#include <eosio/eosio.hpp>
#include <eosio/permission.hpp>
#include "/opt/eosio.contracts/contracts/eosio.token/include/eosio.token/eosio.token.hpp"


class [[eosio::contract("travelrefund")]] travelrefund : public eosio::contract {
    private:
        eosio::name admin_user = eosio::name("trfadminuser");

        // Request
            struct [[eosio::table]] RequestStruct{
                eosio::name user;
                std::string status;
                int distance;
                uint64_t primary_key() const { return user.value; }
            };

            typedef eosio::multi_index<eosio::name("requests"), RequestStruct> RequestMultiIndex;
            typedef eosio::multi_index<eosio::name("requests"), RequestStruct>::const_iterator RequestIterator;

        // Payout
            struct [[eosio::table]] PayoutStruct{
                eosio::name user;
                std::string status;
                int distance;
                double distance_percent; 
                int64_t amount;
                uint64_t primary_key() const { return user.value; }
            };
            typedef eosio::multi_index<eosio::name("payouts"), PayoutStruct> PayoutMultiIndex;
            typedef eosio::multi_index<eosio::name("payouts"), PayoutStruct>::const_iterator PayoutIterator;

        // for debugging
		void log_(const char * memo_char) {
			std::string memo{memo_char};
			eosio::name contract_name{"travelrefund"};
			eosio::action(
				eosio::permission_level{contract_name, eosio::name{"active"}},
				contract_name,
				eosio::name{"log"},
				std::make_tuple(memo)
		   ).send();
		}
        
    public:
        travelrefund(eosio::name self, eosio::name first_receiver, eosio::datastream<const char *> ds) : 
            eosio::contract{self, first_receiver, ds} { }

        // this needs to be here for the internal _log to work
        [[eosio::action]]
        void log( std::string message) {
            // this function intentionally left blank
        }

        [[eosio::action]]
        void create ( eosio::name user) {
            require_auth( user);
            RequestMultiIndex requests{ get_self(), get_first_receiver().value };
            RequestIterator iterator = requests.find(user.value);

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
        void approve(eosio::name user, int distance) {
            require_auth(admin_user);
            if (distance > 0){
                RequestMultiIndex requests{ get_self(), get_first_receiver().value};
                RequestIterator request_iterator = requests.find(user.value);
                if (request_iterator != requests.end()) {
                    requests.modify(request_iterator, admin_user, [&]( RequestStruct& row ) {
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
        void process() {
            require_auth(admin_user);

            // get total distance
            RequestMultiIndex requests = RequestMultiIndex(get_self(), get_first_receiver().value);
            int sum_distance = 0;
			int approved_request_count = 0;
            for (const RequestStruct &request_item : requests) {
                if (request_item.status == "approved") {
                    sum_distance += request_item.distance;
					++approved_request_count;
                }
            }

            // get balance
            int64_t contract_balance = 0;
            {
                eosio::asset balance_asset = eosio::token::get_balance(
                        eosio::name{"eosio.token"},
                        eosio::name{"travelrefund"}, 
                        eosio::symbol_code{"EOS"}
                );
                contract_balance = balance_asset.amount;

				// log
				char memo_char[40];
				sprintf(memo_char,"contract_balance: %lld", (long long)contract_balance); 
				log_(memo_char);
            }


            // a little struct to extract what we need from the request table
            struct RequestForPayout{ // 16 bytes
                eosio::name user;  // 8 bytes
                int distance; // 4 bytes
            };

			// Note: if max_requests is too big, this will crash deployment with a memory violation
			// 3,000,000 RequestForPayouts = 36,000,000 bytes = failed to allocate pages
			eosio::check(approved_request_count < (int)(33600000 /sizeof(RequestForPayout)), "too many approved requests, won't be able to allocate memory");
            RequestForPayout * approved_requests = (RequestForPayout *)malloc(approved_request_count * sizeof(RequestForPayout));

            int request_for_payout_count= 0;
            for (const RequestStruct &request_item : requests) {
                // erase payout if exists
                if (request_item.status == "approved") {
                    int request_idx = request_for_payout_count;
                    approved_requests[request_idx].user = request_item.user;
                    approved_requests[request_idx].distance = request_item.distance;
                    ++request_idx;
                    // error out if we pass max_requests_for_payout
                    eosio::check(request_idx <= approved_request_count, "approved count is greater somehow!");
                    request_for_payout_count = request_idx;
					// I could inline the payout code here, but by moving it out, it's clearer
					// easier to build on, and we can iterate through it for updating the requests 
					// table

                } // approved
            } // for

            // clear out payout table
            PayoutMultiIndex payouts( get_self(), get_first_receiver().value );
            for(PayoutIterator itr = payouts.begin(); itr != payouts.end();) {
                itr = payouts.erase(itr);
            }


            // add approved requests to payout
            double total_percent = 0;
            int64_t total_amount = 0;
            for (int request_idx = 0; request_idx < request_for_payout_count; ++request_idx) {
                RequestForPayout *approved_request = &approved_requests[request_idx];
                PayoutIterator payout_found = payouts.find(approved_request->user.value);
                if (payout_found == payouts.end()) {
                    payouts.emplace(admin_user, [&]( PayoutStruct& row ) {
                        row.user                = approved_request->user;
                        row.status 	            = "processing";
                        row.distance            = approved_request->distance;
                        double percent          = ((double) approved_request->distance / sum_distance) * 100;
                        total_percent          += percent;
                        row.distance_percent    = percent;
                        double percent_decimal = row.distance_percent / 100;
                        // rounded version
                        // don't use: int64_t amount          = (int64_t)(contract_balance * percent_decimal + 0.5); 
                        // Note: not rounding. amount gets cut off here.
                        // Without this, you can get greater than 100%, which will total greater
                        // the balance of the contract account, causing a silent failure
                        int64_t amount          = (int64_t)(contract_balance * (row.distance_percent/ 100));
                        total_amount           += amount;
                        row.amount              = amount;
                    });
                }
            }

            // log total percent to confirm nothing weird is happening
            {
                char memo_char[60];
                sprintf(memo_char,"total_percent: %f", total_percent); 
                log_(memo_char);
            }
            // log total amount to confirm nothing weird is happening
            {
                char memo_char[60];
                sprintf(memo_char,"total_amount: %lld", total_amount); 
                log_(memo_char);
            }
        }

        [[eosio::action]]
        void disburse() {
            eosio::name from{"travelrefund"};
            require_auth(from);

            eosio::symbol eos_symbol{eosio::symbol_code{"EOS"}, 4};

            PayoutMultiIndex payouts( get_self(), get_first_receiver().value );
            char memo_raw[40];
            eosio::transaction txn{};
            for (const PayoutStruct &item : payouts) {
                if (item.status == "processing") {
                    eosio::name to{item.user}; 
                    sprintf(memo_raw, "amount send %lld", item.amount);
                    std::string memo{memo_raw};
                    //eosio::asset amount_to_send{0,eos_symbol};
                    eosio::asset amount_to_send{item.amount,eos_symbol};
                    eosio::action transfer = eosio::action(
                        eosio::permission_level{from, eosio::name{"active"}},
                        eosio::name{"eosio.token"},
                        eosio::name{"transfer"},
                        std::make_tuple(from, to, amount_to_send, memo)
                   );
                   txn.actions.emplace_back(transfer);
                }
            }
            txn.send(0, _self, false);
            // this will run even if the transaction fails.
        }

        [[eosio::action]]
        void reject ( eosio::name user) {
            require_auth(admin_user);
			RequestMultiIndex requests{get_self(), get_first_receiver().value};
			RequestIterator request_iterator = requests.find(user.value);
			if (request_iterator != requests.end()) {
				requests.modify(request_iterator, admin_user, [&]( RequestStruct& row ) {
					row.status 			= "rejected";
				});
			} else {
				printf("\nuser not found\n");
			}
        }

        [[eosio::action]]
        void erase( eosio::name user) {
            require_auth(admin_user);

            // clear out request table
            {
                RequestMultiIndex requests{get_self(), get_first_receiver().value};
                RequestIterator request_iterator = requests.find(user.value);
                if (request_iterator != requests.end()) {
                    requests.erase(request_iterator);
                } else {
                    printf("user not found (already erased?)");
                }
            }

#if 0
            // clear out payout table, probably only want to do this when testing
            PayoutMultiIndex payouts( get_self(), get_first_receiver().value );
			PayoutIterator payout_iterator = payouts.find(user.value);
            if (payout_iterator != payouts.end()) {
                payouts.erase(payout_iterator);
            } else {
                printf("user not found (already erased?)");
            }
#endif
        }

        [[eosio::action]]
        void complete( eosio::name user) {
            require_auth(admin_user);

            {
                RequestMultiIndex requests{get_self(), get_first_receiver().value};
                RequestIterator request_iterator = requests.find(user.value);
                if (request_iterator != requests.end()) {
                    requests.modify(request_iterator, admin_user, [&]( RequestStruct& row ) {
                        row.status 			= "paid";
                    });
                } else {
                    printf("user not found (already erased?)");
                }
            }

            PayoutMultiIndex payouts( get_self(), get_first_receiver().value );
			PayoutIterator payout_iterator = payouts.find(user.value);
            if (payout_iterator != payouts.end()) {
                payouts.modify(payout_iterator, admin_user, [&]( PayoutStruct& row ) {
                    row.status 			= "paid";
                });
            } else {
                printf("user not found (already erased?)");
            }
        }

};

