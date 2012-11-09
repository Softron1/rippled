#ifndef __TRANSACTION__
#define __TRANSACTION__

//
// Notes: this code contains legacy constructored sharedXYZ and setXYZ. The intent is for these functions to go away. Transactions
// should now be constructed in JSON with. Use STObject::parseJson to obtain a binary version.
//

#include <vector>

#include <boost/shared_ptr.hpp>
#include <boost/enable_shared_from_this.hpp>
#include <boost/cstdint.hpp>

#include "../json/value.h"

#include "key.h"
#include "uint256.h"
#include "ripple.pb.h"
#include "Serializer.h"
#include "SHAMap.h"
#include "SerializedTransaction.h"
#include "TransactionErr.h"
#include "InstanceCounter.h"

class Database;

enum TransStatus
{
	NEW			= 0, // just received / generated
	INVALID		= 1, // no valid signature, insufficient funds
	INCLUDED	= 2, // added to the current ledger
	CONFLICTED	= 3, // losing to a conflicting transaction
	COMMITTED	= 4, // known to be in a ledger
	HELD		= 5, // not valid now, maybe later
	REMOVED		= 6, // taken out of a ledger
	OBSOLETE	= 7, // a compatible transaction has taken precedence
	INCOMPLETE	= 8  // needs more signatures
};

DEFINE_INSTANCE(Transaction);

// This class is for constructing and examining transactions.  Transactions are static so manipulation functions are unnecessary.
class Transaction : public boost::enable_shared_from_this<Transaction>, private IS_INSTANCE(Transaction)
{
public:
	typedef boost::shared_ptr<Transaction> pointer;

private:
	uint256			mTransactionID;
	RippleAddress	mAccountFrom;
	RippleAddress	mFromPubKey;	// Sign transaction with this. mSignPubKey
	RippleAddress	mSourcePrivate;	// Sign transaction with this.

	uint32			mInLedger;
	TransStatus		mStatus;
	TER				mResult;

	SerializedTransaction::pointer mTransaction;

	Transaction::pointer setAccountSet(
		const RippleAddress&				naPrivateKey,
		bool								bEmailHash,
		const uint128&						uEmailHash,
		bool								bWalletLocator,
		const uint256&						uWalletLocator,
		const uint32						uWalletSize,
		const RippleAddress&				naMessagePublic,
		bool								bDomain,
		const std::vector<unsigned char>&	vucDomain,
		bool								bTransferRate,
		const uint32						uTransferRate);

	Transaction::pointer setClaim(
		const RippleAddress&				naPrivateKey,
		const std::vector<unsigned char>&	vucGenerator,
		const std::vector<unsigned char>&	vucPubKey,
		const std::vector<unsigned char>&	vucSignature);

	Transaction::pointer setCreate(
		const RippleAddress&				naPrivateKey,
		const RippleAddress&				naCreateAccountID,
		const STAmount&						saFund);

	Transaction::pointer setCreditSet(
		const RippleAddress&				naPrivateKey,
		const STAmount&						saLimitAmount,
		bool								bQualityIn,
		uint32								uQualityIn,
		bool								bQualityOut,
		uint32								uQualityOut);

	Transaction::pointer setNicknameSet(
		const RippleAddress&				naPrivateKey,
		const uint256&						uNickname,
		bool								bSetOffer,
		const STAmount&						saMinimumOffer);

	Transaction::pointer setOfferCreate(
		const RippleAddress&				naPrivateKey,
		bool								bPassive,
		const STAmount&						saTakerPays,
		const STAmount&						saTakerGets,
		uint32								uExpiration);

	Transaction::pointer setOfferCancel(
		const RippleAddress&				naPrivateKey,
		uint32								uSequence);

	Transaction::pointer setPasswordFund(
		const RippleAddress&				naPrivateKey,
		const RippleAddress&				naDstAccountID);

	Transaction::pointer setPasswordSet(
		const RippleAddress&				naPrivateKey,
		const RippleAddress&				naAuthKeyID,
		const std::vector<unsigned char>&	vucGenerator,
		const std::vector<unsigned char>&	vucPubKey,
		const std::vector<unsigned char>&	vucSignature);

	Transaction::pointer setPayment(
		const RippleAddress&				naPrivateKey,
		const RippleAddress&				naDstAccountID,
		const STAmount&						saAmount,
		const STAmount&						saSendMax,
		const STPathSet&					spsPaths,
		const bool							bPartial,
		const bool							bLimit);

	Transaction::pointer setWalletAdd(
		const RippleAddress&				naPrivateKey,
		const STAmount&						saAmount,
		const RippleAddress&				naAuthKeyID,
		const RippleAddress&				naNewPubKey,
		const std::vector<unsigned char>&	vucSignature);

public:
	Transaction(SerializedTransaction::ref st, bool bValidate);

	static Transaction::pointer sharedTransaction(const std::vector<unsigned char>&vucTransaction, bool bValidate);
	static Transaction::pointer transactionFromSQL(Database* db, bool bValidate);

	Transaction(
		TransactionType ttKind,
		const RippleAddress&	naPublicKey,		// To prove transaction is consistent and authorized.
		const RippleAddress&	naSourceAccount,	// To identify the paying account.
		uint32					uSeq,				// To order transactions.
		const STAmount&			saFee,				// Transaction fee.
		uint32					uSourceTag);		// User call back value.

	// Change account settings.
	static Transaction::pointer sharedAccountSet(
		const RippleAddress& naPublicKey, const RippleAddress& naPrivateKey,
		const RippleAddress&				naSourceAccount,
		uint32								uSeq,
		const STAmount&						saFee,
		uint32								uSourceTag,
		bool								bEmailHash,
		const uint128&						uEmailHash,
		bool								bWalletLocator,
		const uint256&						uWalletLocator,
		const uint32						uWalletSize,
		const RippleAddress&				naMessagePublic,
		bool								bDomain,
		const std::vector<unsigned char>&	vucDomain,
		bool								bTransferRate,
		const uint32						uTransferRate);

	// Claim a wallet.
	static Transaction::pointer sharedClaim(
		const RippleAddress& naPublicKey, const RippleAddress& naPrivateKey,
		uint32								uSourceTag,
		const std::vector<unsigned char>&	vucGenerator,
		const std::vector<unsigned char>&	vucPubKey,
		const std::vector<unsigned char>&	vucSignature);

	// Create an account.
	static Transaction::pointer sharedCreate(
		const RippleAddress& naPublicKey, const RippleAddress& naPrivateKey,
		const RippleAddress&				naSourceAccount,
		uint32								uSeq,
		const STAmount&						saFee,
		uint32								uSourceTag,
		const RippleAddress&				naCreateAccountID,	// Account to create.
		const STAmount&						saFund);			// Initial funds in XNC.

	// Set credit limit and borrow fees.
	static Transaction::pointer sharedCreditSet(
		const RippleAddress& naPublicKey, const RippleAddress& naPrivateKey,
		const RippleAddress&				naSourceAccount,
		uint32								uSeq,
		const STAmount&						saFee,
		uint32								uSourceTag,
		const STAmount&						saLimitAmount,
		bool								bQualityIn,
		uint32								uQualityIn,
		bool								bQualityOut,
		uint32								uQualityOut);

	// Set Nickname
	static Transaction::pointer sharedNicknameSet(
		const RippleAddress& naPublicKey, const RippleAddress& naPrivateKey,
		const RippleAddress&				naSourceAccount,
		uint32								uSeq,
		const STAmount&						saFee,
		uint32								uSourceTag,
		const uint256&						uNickname,
		bool								bSetOffer,
		const STAmount&						saMinimumOffer);

	// Pre-fund password change.
	static Transaction::pointer sharedPasswordFund(
		const RippleAddress& naPublicKey, const RippleAddress& naPrivateKey,
		const RippleAddress&				naSourceAccount,
		uint32								uSeq,
		const STAmount&						saFee,
		uint32								uSourceTag,
		const RippleAddress&				naDstAccountID);

	// Change a password.
	static Transaction::pointer sharedPasswordSet(
		const RippleAddress& naPublicKey, const RippleAddress& naPrivateKey,
		uint32								uSourceTag,
		const RippleAddress&				naAuthKeyID,	// ID of regular public to auth.
		const std::vector<unsigned char>&	vucGenerator,
		const std::vector<unsigned char>&	vucPubKey,
		const std::vector<unsigned char>&	vucSignature);

	// Make a payment.
	static Transaction::pointer sharedPayment(
		const RippleAddress& naPublicKey, const RippleAddress& naPrivateKey,
		const RippleAddress&				naSourceAccount,
		uint32								uSeq,
		const STAmount&						saFee,
		uint32								uSourceTag,
		const RippleAddress&				naDstAccountID,
		const STAmount&						saAmount,
		const STAmount&						saSendMax,
		const STPathSet&					spsPaths,
		const bool							bPartial = false,
		const bool							bLimit = false);

	// Place an offer.
	static Transaction::pointer sharedOfferCreate(
		const RippleAddress& naPublicKey, const RippleAddress& naPrivateKey,
		const RippleAddress&				naSourceAccount,
		uint32								uSeq,
		const STAmount&						saFee,
		uint32								uSourceTag,
		bool								bPassive,
		const STAmount&						saTakerPays,
		const STAmount&						saTakerGets,
		uint32								uExpiration);

	// Cancel an offer
	static Transaction::pointer sharedOfferCancel(
		const RippleAddress& naPublicKey, const RippleAddress& naPrivateKey,
		const RippleAddress&				naSourceAccount,
		uint32								uSeq,
		const STAmount&						saFee,
		uint32								uSourceTag,
		uint32								uSequence);

	// Add an account to a wallet.
	static Transaction::pointer sharedWalletAdd(
		const RippleAddress& naPublicKey, const RippleAddress& naPrivateKey,
		const RippleAddress&				naSourceAccount,
		uint32								uSeq,
		const STAmount&						saFee,
		uint32								uSourceTag,
		const STAmount&						saAmount,		// Initial funds in XNC.
		const RippleAddress&				naAuthKeyID,	// ID of regular public to auth.
		const RippleAddress&				naNewPubKey,	// Public key of new account
		const std::vector<unsigned char>&	vucSignature);	// Proof know new account's private key.

	bool sign(const RippleAddress& naAccountPrivate);
	bool checkSign() const;
	void updateID() { mTransactionID=mTransaction->getTransactionID(); }

	SerializedTransaction::pointer getSTransaction() { return mTransaction; }

	const uint256& getID() const					{ return mTransactionID; }
	const RippleAddress& getFromAccount() const		{ return mAccountFrom; }
	STAmount getAmount() const						{ return mTransaction->getFieldU64(sfAmount); }
	STAmount getFee() const							{ return mTransaction->getTransactionFee(); }
	uint32 getFromAccountSeq() const				{ return mTransaction->getSequence(); }
	uint32 getIdent() const							{ return mTransaction->getFieldU32(sfSourceTag); }
	std::vector<unsigned char> getSignature() const	{ return mTransaction->getSignature(); }
	uint32 getLedger() const						{ return mInLedger; }
	TransStatus getStatus() const					{ return mStatus; }

	TER getResult()									{ return mResult; }
	void setResult(TER terResult)					{ mResult = terResult; }

	void setStatus(TransStatus status, uint32 ledgerSeq);
	void setStatus(TransStatus status) { mStatus=status; }
	void setLedger(uint32 ledger) { mInLedger = ledger; }

	// database functions
	static void saveTransaction(const Transaction::pointer&);
	bool save();
	static Transaction::pointer load(const uint256& id);
	static Transaction::pointer findFrom(const RippleAddress& fromID, uint32 seq);

	// conversion function
	static bool convertToTransactions(uint32 ourLedgerSeq, uint32 otherLedgerSeq,
		bool checkFirstTransactions, bool checkSecondTransactions, const SHAMap::SHAMapDiff& inMap,
		std::map<uint256, std::pair<Transaction::pointer, Transaction::pointer> >& outMap);

	bool operator<(const Transaction&) const;
	bool operator>(const Transaction&) const;
	bool operator==(const Transaction&) const;
	bool operator!=(const Transaction&) const;
	bool operator<=(const Transaction&) const;
	bool operator>=(const Transaction&) const;

	Json::Value getJson(int options) const;

	static bool isHexTxID(const std::string&);

protected:
	static Transaction::pointer transactionFromSQL(const std::string& statement);
};

#endif
// vim:ts=4