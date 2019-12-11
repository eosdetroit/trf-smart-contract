
# Tables

**Requests**

```
    eosio::name user; // primary key
    std::string status; // pending, approved, rejected, paid
    int distance;
```

**Payouts**

```
    eosio::name user; // primary key
    std::string status; // processing, paid
    int distance; // same as from requests
    double distance_percent; // percent of total distance
    int64_t amount; // amount to be paid out
```

# Actions

**Create**

> Create a new request. Anyone can do this.
> This is done through trf.eosdetroit.io, for conference attendees.
> The first step on a journey to getting paid, and the only one the conference attendee needs to make.
> This info is saved in the Requests table (user, status = "pending")

```
    void create ( eosio::name user)
```

**Approve**

> Approve an attendee for a certain distance. This can only be done by the admin user.
> trf.eosdetroit.io provides the interface for this, for the admin.
> We take their departing city and calculate the distance to Rio in miles. 
> That info is saved in the Requests table (distance, status = "approved")

```
    void approve(eosio::name user, int distance)
```



**Reject**

> Admin can use this action to reject a spammer, or otherwise invalid account.
> Updates Requests (status = "rejected")

```
    void reject ( eosio::name user)
```



**Process**

> Admin user runs this to generate the Payouts table for verifiation
> Should only be run after all approvals have been made. 
> Calculates the percentage each attendee should receive. 
> This info is saved in the Payouts table (user, status = 'processing', distance, distance_percent, amount)
> The contract balance, total percentage and total amount to be paid out are logged for verification purposes.
> Note that the amount given out will not total up to be the exact balance of the contract - we can only be precise up to 1 one thousandth of an EOS. We drop the remainder (otherwise we would exceed the balance of the account and the transaction would fail in the disburse step below).


```
    void process() 
```



**Disburse**

> Admin runs this after Payouts table has been verified as correct.
> Loops through and pays out the amount noted in "amount" in the Payouts table.
> It uses inline actions for this, and the disburse action can't tell whether or not it has succeeded, so this must be verified after the fact. 


```
    void disburse()

```



**Erase**

> Admin can use this action to erase a user from Requests. 
> Usually used to iterate through and clear out a table for debugging, or individual test users.
> In the debug process, it's useful to erase users from the Payout table too, 
> but we disable this for live usage. 

```
    void erase( eosio::name user)
```



**Complete**

> Admin can use this action to mark a user as paid. This would only be done after manual verification.
> Updates Payouts (status="paid")
> Updates Requests (status="paid")


```
    void complete( eosio::name user) {
```


**Log**

> Used in the contract as an inline action for debugging only
> Allows us to verify amount being sent is correct, by reading bloks.io

```
        void log( std::string message)
```
