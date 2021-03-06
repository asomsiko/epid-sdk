/*############################################################################
  # Copyright 2017-2018 Intel Corporation
  #
  # Licensed under the Apache License, Version 2.0 (the "License");
  # you may not use this file except in compliance with the License.
  # You may obtain a copy of the License at
  #
  #     http://www.apache.org/licenses/LICENSE-2.0
  #
  # Unless required by applicable law or agreed to in writing, software
  # distributed under the License is distributed on an "AS IS" BASIS,
  # WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  # See the License for the specific language governing permissions and
  # limitations under the License.
  ############################################################################*/
/// Tiny member sign implementation.
/*! \file */

#define EXPORT_EPID_APIS
#include <epid/member/api.h>
#include "epid/common/src/sig_types.h"
#include "epid/member/tiny/src/context.h"
#include "epid/member/tiny/src/native_types.h"
#include "epid/member/tiny/src/nrprove.h"
#include "epid/member/tiny/src/serialize.h"
#include "epid/member/tiny/src/signbasic.h"
#include "epid/member/tiny/stdlib/endian.h"
#include "epid/member/tiny/stdlib/tiny_stdlib.h"

// SIZE_MAX is not guaranteed in C89/90
#define SIZE_T_MAX ((size_t)(-1))

size_t EPID_MEMBER_API EpidGetSigSize(SigRl const* sig_rl) {
  const size_t kMinSigSize = sizeof(EpidNonSplitSignature) - sizeof(NrProof);
  if (!sig_rl) {
    return kMinSigSize;
  } else {
    uint32_t n2 = be32toh(sig_rl->n2);
    if (n2 > (SIZE_T_MAX - kMinSigSize) / sizeof(NrProof)) {
      return kMinSigSize;
    } else {
      return kMinSigSize + n2 * sizeof(NrProof);
    }
  }
}

EpidStatus EPID_MEMBER_API EpidSign(MemberCtx const* ctx, void const* msg,
                                    size_t msg_len, void const* basename,
                                    size_t basename_len, EpidSignature* raw_sig,
                                    size_t sig_len) {
  EpidNonSplitSignature* sig = (EpidNonSplitSignature*)raw_sig;
  EpidStatus sts = kEpidErr;
  OctStr32 octstr32_0 = {{0x00, 0x00, 0x00, 0x00}};
  NativeBasicSignature sigma0;
  if (!ctx || !sig) {
    return kEpidBadArgErr;
  }
  if (!msg && (0 != msg_len)) {
    // if message is non-empty it must have both length and content
    return kEpidBadArgErr;
  }
  if (!ctx->is_provisioned) {
    return kEpidOutOfSequenceError;
  }
  if (!basename && (0 != basename_len)) {
    // if basename is non-empty it must have both length and content
    return kEpidBadArgErr;
  }
  if (EpidGetSigSize(ctx->sig_rl) > sig_len) {
    return kEpidBadArgErr;
  }

  // 11. The member sets sigma0 = (B, K, T, c, sx, sf, sa, sb).
  sts = EpidSignBasic(ctx, msg, msg_len, basename, basename_len, &sigma0);
  if (kEpidNoErr != sts) {
    return sts;
  }
  BasicSignatureSerialize(&sig->sigma0, &sigma0);
  if (ctx->sig_rl) {
    uint32_t i = 0;
    uint32_t num_sig_rl = 0;
    EpidStatus nr_prove_status = kEpidNoErr;
    // 13. If SigRL is provided as input, the member proceeds with
    //     the following steps:
    //   a. The member verifies that gid in public key and in SigRL
    //      match.
    //      This was done under EpidMemberSetSigRl function.
    //   b. The member copies RLver and n2 values in SigRL to the
    //      signature.
    sig->rl_ver = ctx->sig_rl->version;
    sig->n2 = ctx->sig_rl->n2;
    //   c. For i = 0, ..., n2-1, the member computes sigma[i] =
    //      nrProve(f, B, K, B[i], K[i]). The details of nrProve()
    //      will be given in the next subsection.
    num_sig_rl = be32toh(ctx->sig_rl->n2);
    for (i = 0; i < num_sig_rl; i++) {
      sts = EpidNrProve(ctx, msg, msg_len, &sigma0, &ctx->sig_rl->bk[i],
                        &sig->sigma[i]);
      if (kEpidNoErr != sts) {
        nr_prove_status = sts;
      }
    }
    if (kEpidNoErr != nr_prove_status) {
      return nr_prove_status;
    }
  } else {
    // 12. If SigRL is not provided as input,
    //   a. The member sets RLver = 0 and n2 = 0.
    //   b. The member outputs (sigma0, RLver, n2) and returns "succeeded".
    sig->rl_ver = octstr32_0;
    sig->n2 = octstr32_0;
  }
  //   d. The member outputs (sigma0, RLver, n2, sigma[0], ...,
  //      sigma[n2-1]).
  //   e. If any of the nrProve() functions outputs "failed", the
  //      member returns "revoked", otherwise returns "succeeded".
  return kEpidNoErr;
}
