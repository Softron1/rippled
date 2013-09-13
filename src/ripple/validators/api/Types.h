//------------------------------------------------------------------------------
/*
    Copyright (c) 2011-2013, OpenCoin, Inc.
*/
//==============================================================================

#ifndef RIPPLE_VALIDATORS_TYPES_H_INCLUDED
#define RIPPLE_VALIDATORS_TYPES_H_INCLUDED

namespace Validators
{

typedef RipplePublicKey     PublicKey;
typedef RipplePublicKeyHash PublicKeyHash;

struct ReceivedValidation
{
    uint256 ledgerHash;
    PublicKeyHash signerPublicKeyHash;
};

}

#endif