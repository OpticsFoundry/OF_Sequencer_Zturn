/*
 * OpticsFoundry trial boot of a new primary boot image (2026-11-27), FSBL part.
 * The scheme is described in OpticsFoundry_Seq_Zturn_App/src/trial_boot.h.
 */
#ifndef OF_TRIAL_BOOT_H
#define OF_TRIAL_BOOT_H

/* Call in QSPI boot mode after InitQspi(). Starts the trial boot watchdog, falls back to the backup image
 * (by a software reset, doesn't return) or does nothing. */
void OfTrialBoot_Check(void);

#endif
