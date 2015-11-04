//------------------------------------------------------------------------------
// File: ISynth.h
//
// Desc: DirectShow sample code - custom interface to allow the user to
//       adjust the frequency.
//
// Copyright (c) Microsoft Corporation.  All rights reserved.
//------------------------------------------------------------------------------


#ifndef __ISYNTH2__
#define __ISYNTH2__

#ifdef __cplusplus
extern "C" {
#endif


//
// ISynth2's GUID
//
// {00487A78-D875-44b0-ADBB-DECA9CDB51FC}
DEFINE_GUID(IID_ISynth2, 
0x487a78, 0xd875, 0x44b0, 0xad, 0xbb, 0xde, 0xca, 0x9c, 0xdb, 0x51, 0xfc);


//
// ISynth2
//
DECLARE_INTERFACE_(ISynth2, IUnknown) {

    STDMETHOD(Set_TimeInterval) (THIS_
                  unsigned int 
             ) PURE;
    
};


#ifdef __cplusplus
}
#endif

#endif // __ISYNTH2__


