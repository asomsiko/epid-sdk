/*############################################################################
  # Copyright 2016-2018 Intel Corporation
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

/*!
 * \file
 * \brief Signature verification implementation.
 */

#include "src/verifysig.h"

#include <stdlib.h>

#include "epid/common/file_parser.h"
#include "epid/verifier/api.h"

EpidStatus Verify(EpidSignature const* sig, size_t sig_len, void const* msg,
                  size_t msg_len, void const* basename, size_t basename_len,
                  void const* signed_priv_rl, size_t signed_priv_rl_size,
                  void const* signed_sig_rl, size_t signed_sig_rl_size,
                  void const* signed_grp_rl, size_t signed_grp_rl_size,
                  VerifierRl const* ver_rl, size_t ver_rl_size,
                  void const* signed_pub_key, size_t signed_pub_key_size,
                  EpidCaCertificate const* cacert, void** verifier_precomp,
                  size_t* verifier_precomp_size) {
  EpidStatus result = kEpidErr;
  VerifierCtx* ctx = NULL;
  PrivRl* priv_rl = NULL;
  SigRl* sig_rl = NULL;
  GroupRl* grp_rl = NULL;

  do {
    GroupPubKey pub_key = {0};
    // authenticate and extract group public key
    result = EpidParseGroupPubKeyFile(signed_pub_key, signed_pub_key_size,
                                      cacert, &pub_key);
    if (kEpidNoErr != result) {
      break;
    }
    // ensure the pre-computation blob is not in a legacy format
    if (*verifier_precomp &&
        *verifier_precomp_size != sizeof(VerifierPrecomp)) {
      result = kEpidBadArgErr;
      break;
    }
    *verifier_precomp_size = sizeof(VerifierPrecomp);

    // create verifier
    result = EpidVerifierCreate(&pub_key, *verifier_precomp, &ctx);
    if (kEpidNoErr != result) {
      break;
    }

    // serialize verifier pre-computation blob
    if (!*verifier_precomp) {
      *verifier_precomp = calloc(1, *verifier_precomp_size);
    }
    result = EpidVerifierWritePrecomp(ctx, *verifier_precomp);
    if (kEpidNoErr != result) {
      break;
    }

    // set the basename used for signing
    result = EpidVerifierSetBasename(ctx, basename, basename_len);
    if (kEpidNoErr != result) {
      break;
    }

    if (signed_priv_rl) {
      // authenticate and determine space needed for RL
      size_t priv_rl_size = 0;
      result = EpidParsePrivRlFile(signed_priv_rl, signed_priv_rl_size, cacert,
                                   NULL, &priv_rl_size);
      if (kEpidNoErr != result) {
        break;
      }

      priv_rl = calloc(1, priv_rl_size);
      if (!priv_rl) {
        result = kEpidMemAllocErr;
        break;
      }

      // fill the rl
      result = EpidParsePrivRlFile(signed_priv_rl, signed_priv_rl_size, cacert,
                                   priv_rl, &priv_rl_size);
      if (kEpidNoErr != result) {
        break;
      }

      // set private key based revocation list
      result = EpidVerifierSetPrivRl(ctx, priv_rl, priv_rl_size);
      if (kEpidNoErr != result) {
        break;
      }
    }  // if (signed_priv_rl)

    if (signed_sig_rl) {
      // authenticate and determine space needed for RL
      size_t sig_rl_size = 0;
      result = EpidParseSigRlFile(signed_sig_rl, signed_sig_rl_size, cacert,
                                  NULL, &sig_rl_size);
      if (kEpidNoErr != result) {
        break;
      }

      sig_rl = calloc(1, sig_rl_size);
      if (!sig_rl) {
        result = kEpidMemAllocErr;
        break;
      }

      // fill the rl
      result = EpidParseSigRlFile(signed_sig_rl, signed_sig_rl_size, cacert,
                                  sig_rl, &sig_rl_size);
      if (kEpidNoErr != result) {
        break;
      }

      // set signature based revocation list
      result = EpidVerifierSetSigRl(ctx, sig_rl, sig_rl_size);
      if (kEpidNoErr != result) {
        break;
      }
    }  // if (signed_sig_rl)

    if (signed_grp_rl) {
      // authenticate and determine space needed for RL
      size_t grp_rl_size = 0;
      result = EpidParseGroupRlFile(signed_grp_rl, signed_grp_rl_size, cacert,
                                    NULL, &grp_rl_size);
      if (kEpidNoErr != result) {
        break;
      }

      grp_rl = calloc(1, grp_rl_size);
      if (!grp_rl) {
        result = kEpidMemAllocErr;
        break;
      }

      // fill the rl
      result = EpidParseGroupRlFile(signed_grp_rl, signed_grp_rl_size, cacert,
                                    grp_rl, &grp_rl_size);
      if (kEpidNoErr != result) {
        break;
      }
      // set group revocation list
      result = EpidVerifierSetGroupRl(ctx, grp_rl, grp_rl_size);
      if (kEpidNoErr != result) {
        break;
      }
    }  // if (signed_grp_rl)

    if (ver_rl) {
      // set verifier based revocation list
      result = EpidVerifierSetVerifierRl(ctx, ver_rl, ver_rl_size);
      if (kEpidNoErr != result) {
        break;
      }
    }

    // verify signature
    result = EpidVerify(ctx, sig, sig_len, msg, msg_len);
    if (kEpidNoErr != result) {
      break;
    }
  } while (0);

  // delete verifier
  EpidVerifierDelete(&ctx);

  if (priv_rl) free(priv_rl);
  if (sig_rl) free(sig_rl);
  if (grp_rl) free(grp_rl);

  return result;
}
