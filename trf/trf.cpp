#include <eosio/eosio.hpp>

class [[eosio::contract("trf")]] trf : public eosio::contract {
    public:
        trf(eosio::name self, eosio::name first_receiver, eosio::datastream<const char *> ds) : eosio::contract(self, first_receiver, ds) {
            /*
            print( "self: ", self, ' ');
            print( "first_receiver: ", first_receiver, ' ');
            char * ds_string = (char *)malloc(100);
            ds.seekp(0);
            int len = ds.remaining();
            ds.read(ds_string, len);
                printf( "ds: '");
            for (int i = 0; i < len; ++i) {
                printf( "%i,", ds_string[i] );
            }
            printf("' ");
            */
        }


        [[eosio::action]]
        void create ( eosio::name user) {
            require_auth( user);
            request_index requests( get_self(), get_first_receiver().value );
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
        void approve(eosio::name user, int distance) {
            require_auth(admin_user);
            if (distance > 0){
                print("Approve request for ", user);
                request_index requests( get_self(), get_first_receiver().value);
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
			request_index requests( get_self(), get_first_receiver().value);
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
			request_index requests( get_self(), get_first_receiver().value);

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
        eosio::name admin_user = eosio::name("bob");
        struct [[eosio::table]] request{
            eosio::name user;
            std::string status;
            int distance;
            uint64_t primary_key() const { return user.value; }
        };
        typedef eosio::multi_index<eosio::name("requests"), request> request_index;
        
};

