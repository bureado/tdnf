/*
 * Copyright (C) 2015-2021 VMware, Inc. All Rights Reserved.
 *
 * Licensed under the GNU General Public License v2 (the "License");
 * you may not use this file except in compliance with the License. The terms
 * of the License are located in the COPYING file of this distribution.
 */

/*
 * Module   : installcmd.c
 *
 * Abstract :
 *
 *            tdnf
 *
 *            command line tool
 *
 * Authors  : Priyesh Padmavilasom (ppadmavilasom@vmware.com)
 *
 */

#include "includes.h"

uint32_t
TDNFCliInstallCommand(
    PTDNF_CLI_CONTEXT pContext,
    PTDNF_CMD_ARGS pCmdArgs
    )
{
    uint32_t dwError = 0;

    dwError = TDNFCliAlterCommand(pContext, pCmdArgs, ALTER_INSTALL);
    BAIL_ON_CLI_ERROR(dwError);

cleanup:
    return dwError;

error:
    goto cleanup;
}

uint32_t
TDNFCliEraseCommand(
    PTDNF_CLI_CONTEXT pContext,
    PTDNF_CMD_ARGS pCmdArgs
    )
{
    uint32_t dwError = 0;

    dwError = TDNFCliAlterCommand(pContext, pCmdArgs, ALTER_ERASE);
    BAIL_ON_CLI_ERROR(dwError);

cleanup:
    return dwError;

error:
    goto cleanup;
}

uint32_t
TDNFCliUpgradeCommand(
    PTDNF_CLI_CONTEXT pContext,
    PTDNF_CMD_ARGS pCmdArgs
    )
{
    uint32_t dwError = 0;
    int nAlterType = ALTER_UPGRADE;

    if(pCmdArgs->nCmdCount == 1)
    {
        nAlterType = ALTER_UPGRADEALL;
    }

    dwError = TDNFCliAlterCommand(pContext, pCmdArgs, nAlterType);
    BAIL_ON_CLI_ERROR(dwError);

cleanup:
    return dwError;

error:
    goto cleanup;
}

uint32_t
TDNFCliDistroSyncCommand(
    PTDNF_CLI_CONTEXT pContext,
    PTDNF_CMD_ARGS pCmdArgs
    )
{
    uint32_t dwError = 0;

    dwError = TDNFCliAlterCommand(pContext, pCmdArgs, ALTER_DISTRO_SYNC);
    BAIL_ON_CLI_ERROR(dwError);

cleanup:
    return dwError;

error:
    goto cleanup;
}

uint32_t
TDNFCliDowngradeCommand(
    PTDNF_CLI_CONTEXT pContext,
    PTDNF_CMD_ARGS pCmdArgs
    )
{
    uint32_t dwError = 0;
    int nAlterType = ALTER_DOWNGRADE;

    if(pCmdArgs->nCmdCount == 1)
    {
        nAlterType = ALTER_DOWNGRADEALL;
    }

    dwError = TDNFCliAlterCommand(pContext, pCmdArgs, nAlterType);
    BAIL_ON_CLI_ERROR(dwError);

cleanup:
    return dwError;

error:
    goto cleanup;
}

uint32_t
TDNFCliAutoEraseCommand(
    PTDNF_CLI_CONTEXT pContext,
    PTDNF_CMD_ARGS pCmdArgs
    )
{
    uint32_t dwError = 0;

    dwError = TDNFCliAlterCommand(pContext, pCmdArgs, ALTER_AUTOERASE);
    BAIL_ON_CLI_ERROR(dwError);

cleanup:
    return dwError;

error:
    goto cleanup;
}

uint32_t
TDNFCliReinstallCommand(
    PTDNF_CLI_CONTEXT pContext,
    PTDNF_CMD_ARGS pCmdArgs
    )
{
    uint32_t dwError = 0;

    dwError = TDNFCliAlterCommand(pContext, pCmdArgs, ALTER_REINSTALL);
    BAIL_ON_CLI_ERROR(dwError);

cleanup:
    return dwError;

error:
    goto cleanup;
}

uint32_t
TDNFCliAlterCommand(
    PTDNF_CLI_CONTEXT pContext,
    PTDNF_CMD_ARGS pCmdArgs,
    TDNF_ALTERTYPE nAlterType
    )
{
    uint32_t dwError = 0;
    char** ppszPackageArgs = NULL;
    int nPackageCount = 0;
    PTDNF_SOLVED_PKG_INFO pSolvedPkgInfo = NULL;
    int nSilent = 0;

    if(!pContext || !pContext->hTdnf || !pCmdArgs)
    {
        dwError = ERROR_TDNF_INVALID_PARAMETER;
        BAIL_ON_CLI_ERROR(dwError);
    }

    nSilent = pCmdArgs->nNoOutput;

    dwError = TDNFCliParsePackageArgs(
                  pCmdArgs,
                  &ppszPackageArgs,
                  &nPackageCount);
    BAIL_ON_CLI_ERROR(dwError);

    dwError = pContext->pFnResolve(
                  pContext,
                  nAlterType,
                  &pSolvedPkgInfo);
    BAIL_ON_CLI_ERROR(dwError);

    if(!nSilent && pSolvedPkgInfo->ppszPkgsNotResolved)
    {
        dwError = PrintNotAvailable(pSolvedPkgInfo->ppszPkgsNotResolved);
        BAIL_ON_CLI_ERROR(dwError);
    }

    if(!pSolvedPkgInfo->nNeedAction)
    {
        dwError = ERROR_TDNF_CLI_NOTHING_TO_DO;
        //If there are unresolved, error with no match
        if(pSolvedPkgInfo->ppszPkgsNotResolved &&
           *pSolvedPkgInfo->ppszPkgsNotResolved)
        {
            dwError = ERROR_TDNF_NO_MATCH;
        }
        BAIL_ON_CLI_ERROR(dwError);
    }

    if(!nSilent)
    {
        dwError = PrintSolvedInfo(pSolvedPkgInfo);
        if (pCmdArgs->nDownloadOnly)
        {
            pr_info("tdnf will only download packages needed for the transaction\n");
        }
    }

    if(pSolvedPkgInfo->nNeedAction)
    {
        int nAnswer = 0;

        dwError = TDNFYesOrNo(pCmdArgs, "Is this ok [y/N]: ", &nAnswer);
        BAIL_ON_CLI_ERROR(dwError);

        if(nAnswer)
        {
            if(!nSilent && pSolvedPkgInfo->nNeedDownload)
            {
                pr_info("\nDownloading:\n");
            }

            dwError = pContext->pFnAlter(
                          pContext,
                          nAlterType,
                          pSolvedPkgInfo);
            BAIL_ON_CLI_ERROR(dwError);

            if(!nSilent)
            {
                pr_info("\nComplete!\n");
                if (pCmdArgs->nDownloadOnly)
                {
                    if (pCmdArgs->pszDownloadDir != NULL)
                    {
                        pr_info("Packages have been downloaded to %s.\n",
                                pCmdArgs->pszDownloadDir);
                    } else {
                        pr_info("Packages have been downloaded to cache.\n");
                    }
                }
            }
        }
        else
        {
            dwError = ERROR_TDNF_OPERATION_ABORTED;
            BAIL_ON_CLI_ERROR(dwError);
        }
    }

cleanup:
    TDNF_CLI_SAFE_FREE_STRINGARRAY(ppszPackageArgs);
    TDNFCliFreeSolvedPackageInfo(pSolvedPkgInfo);
    return dwError;

error:
    if (dwError == ERROR_TDNF_ALREADY_INSTALLED)
    {
        dwError = ERROR_TDNF_CLI_NOTHING_TO_DO;
    }

    goto cleanup;
}

uint32_t
PrintSolvedInfo(
    PTDNF_SOLVED_PKG_INFO pSolvedPkgInfo
    )
{
    uint32_t dwError = 0;

    if(!pSolvedPkgInfo)
    {
        dwError = ERROR_TDNF_INVALID_PARAMETER;
        BAIL_ON_CLI_ERROR(dwError);
    }

    if(pSolvedPkgInfo->pPkgsExisting)
    {
        dwError = PrintExistingPackagesSkipped(pSolvedPkgInfo->pPkgsExisting);
        BAIL_ON_CLI_ERROR(dwError);
    }
    if(pSolvedPkgInfo->pPkgsNotAvailable)
    {
        dwError = PrintNotAvailablePackages(pSolvedPkgInfo->pPkgsNotAvailable);
        BAIL_ON_CLI_ERROR(dwError);
    }
    if(pSolvedPkgInfo->pPkgsToInstall)
    {
        dwError = PrintAction(pSolvedPkgInfo->pPkgsToInstall, ALTER_INSTALL);
        BAIL_ON_CLI_ERROR(dwError);
    }
    if(pSolvedPkgInfo->pPkgsToUpgrade)
    {
        dwError = PrintAction(pSolvedPkgInfo->pPkgsToUpgrade, ALTER_UPGRADE);
        BAIL_ON_CLI_ERROR(dwError);
    }
    if(pSolvedPkgInfo->pPkgsToDowngrade)
    {
        dwError = PrintAction(
                      pSolvedPkgInfo->pPkgsToDowngrade,
                      ALTER_DOWNGRADE);
        BAIL_ON_CLI_ERROR(dwError);
    }
    if(pSolvedPkgInfo->pPkgsToRemove)
    {
        dwError = PrintAction(pSolvedPkgInfo->pPkgsToRemove, ALTER_ERASE);
        BAIL_ON_CLI_ERROR(dwError);
    }
    if(pSolvedPkgInfo->pPkgsUnNeeded)
    {
        dwError = PrintAction(pSolvedPkgInfo->pPkgsUnNeeded, ALTER_ERASE);
        BAIL_ON_CLI_ERROR(dwError);
    }
    if(pSolvedPkgInfo->pPkgsToReinstall)
    {
        dwError = PrintAction(
                      pSolvedPkgInfo->pPkgsToReinstall,
                      ALTER_REINSTALL);
        BAIL_ON_CLI_ERROR(dwError);
    }
    if(pSolvedPkgInfo->pPkgsObsoleted)
    {
        dwError = PrintAction(pSolvedPkgInfo->pPkgsObsoleted, ALTER_OBSOLETED);
        BAIL_ON_CLI_ERROR(dwError);
    }

cleanup:
    return dwError;

error:
    goto cleanup;
}

uint32_t
PrintNotAvailable(
    char** ppszPkgsNotAvailable
    )
{
    uint32_t dwError = 0;
    int i = 0;
    #define BOLD "\033[1m\033[30m"
    #define RESET   "\033[0m"

    if(!ppszPkgsNotAvailable)
    {
        dwError = ERROR_TDNF_INVALID_PARAMETER;
        BAIL_ON_CLI_ERROR(dwError);
    }

    while(ppszPkgsNotAvailable[i])
    {
        pr_info("No package " BOLD "%s " RESET "available\n",
                ppszPkgsNotAvailable[i]);
        ++i;
    }
cleanup:
    return dwError;
error:
    goto cleanup;
}

uint32_t
PrintExistingPackagesSkipped(
    PTDNF_PKG_INFO pPkgInfos
    )
{
    uint32_t dwError = 0;

    PTDNF_PKG_INFO pPkgInfo = NULL;

    if(!pPkgInfos)
    {
        dwError = ERROR_TDNF_INVALID_PARAMETER;
        BAIL_ON_CLI_ERROR(dwError);
    }

    pPkgInfo = pPkgInfos;
    while(pPkgInfo)
    {
        pr_info(
            "Package %s-%s-%s.%s is already installed, skipping.\n",
            pPkgInfo->pszName,
            pPkgInfo->pszVersion,
            pPkgInfo->pszRelease,
            pPkgInfo->pszArch);
        pPkgInfo = pPkgInfo->pNext;
    }

cleanup:
    return dwError;

error:
    goto cleanup;
}

uint32_t
PrintNotAvailablePackages(
    PTDNF_PKG_INFO pPkgInfos
    )
{
    uint32_t dwError = 0;

    PTDNF_PKG_INFO pPkgInfo = NULL;

    if(!pPkgInfos)
    {
        dwError = ERROR_TDNF_INVALID_PARAMETER;
        BAIL_ON_CLI_ERROR(dwError);
    }

    pPkgInfo = pPkgInfos;
    while(pPkgInfo)
    {
        pr_info(
            "No package %s available.\n",
            pPkgInfo->pszName);
        pPkgInfo = pPkgInfo->pNext;
    }

cleanup:
    return dwError;
error:
    goto cleanup;
}

uint32_t
PrintAction(
    PTDNF_PKG_INFO pPkgInfos,
    TDNF_ALTERTYPE nAlterType
    )
{
    uint32_t dwError = 0;
    PTDNF_PKG_INFO pPkgInfo = NULL;

    uint32_t dwTotalInstallSize = 0;
    char* pszTotalInstallSize = NULL;
    char  *pszEmptyString = "";

    #define COL_COUNT 5
    //Name | Arch | [Epoch:]Version-Release | Repository | Install Size
    int nColPercents[COL_COUNT] = {30, 15, 20, 15, 10};
    int nColWidths[COL_COUNT] = {0};

    #define MAX_COL_LEN 256
    char szEpochVersionRelease[MAX_COL_LEN] = {0};
    char *ppszInfoToPrint[MAX_COL_LEN] = {0};

    if(!pPkgInfos)
    {
        dwError = ERROR_TDNF_INVALID_PARAMETER;
        BAIL_ON_CLI_ERROR(dwError);
    }

    switch(nAlterType)
    {
        case ALTER_INSTALL:
            pr_info("\nInstalling:");
            break;
        case ALTER_UPGRADE:
            pr_info("\nUpgrading:");
            break;
        case ALTER_ERASE:
            pr_info("\nRemoving:");
            break;
        case ALTER_DOWNGRADE:
            pr_info("\nDowngrading:");
            break;
        case ALTER_REINSTALL:
            pr_info("\nReinstalling:");
            break;
        case ALTER_OBSOLETED:
            pr_info("\nObsoleting:");
            break;
        default:
            dwError = ERROR_TDNF_INVALID_PARAMETER;
            BAIL_ON_CLI_ERROR(dwError);
    }
    pr_info("\n");

    dwError = GetColumnWidths(COL_COUNT, nColPercents, nColWidths);
    BAIL_ON_CLI_ERROR(dwError);

    pPkgInfo = pPkgInfos;
    while(pPkgInfo)
    {
        dwTotalInstallSize += pPkgInfo->dwInstallSizeBytes;
        memset(szEpochVersionRelease, 0, MAX_COL_LEN);
        if(pPkgInfo->dwEpoch)
        {
            if(snprintf(
                szEpochVersionRelease,
                MAX_COL_LEN,
                "%u:%s-%s",
                (unsigned)pPkgInfo->dwEpoch,
                pPkgInfo->pszVersion,
                pPkgInfo->pszRelease) < 0)
            {
                dwError = errno;
                BAIL_ON_CLI_ERROR(dwError);
            }
        }
        else
        {
            if(snprintf(
                szEpochVersionRelease,
                MAX_COL_LEN,
                "%s-%s",
                pPkgInfo->pszVersion,
                pPkgInfo->pszRelease) < 0)
            {
                dwError = errno;
                BAIL_ON_CLI_ERROR(dwError);
            }
        }

        ppszInfoToPrint[0] = pPkgInfo->pszName == NULL ?
                                 pszEmptyString : pPkgInfo->pszName;
        ppszInfoToPrint[1] = pPkgInfo->pszArch == NULL ?
                                 pszEmptyString : pPkgInfo->pszArch;
        ppszInfoToPrint[2] = szEpochVersionRelease;
        ppszInfoToPrint[3] = pPkgInfo->pszRepoName == NULL ?
                                 pszEmptyString : pPkgInfo->pszRepoName;
        ppszInfoToPrint[4] = pPkgInfo->pszFormattedSize == NULL ?
                                 pszEmptyString : pPkgInfo->pszFormattedSize;
        pr_info(
            "%-*s %-*s %-*s %-*s %*s\n",
            nColWidths[0],
            ppszInfoToPrint[0],
            nColWidths[1],
            ppszInfoToPrint[1],
            nColWidths[2],
            ppszInfoToPrint[2],
            nColWidths[3],
            ppszInfoToPrint[3],
            nColWidths[4],
            ppszInfoToPrint[4]);
        pPkgInfo = pPkgInfo->pNext;
    }

    TDNFUtilsFormatSize(dwTotalInstallSize, &pszTotalInstallSize);
    pr_info("\nTotal installed size: %s\n", pszTotalInstallSize);

cleanup:
    if (pszTotalInstallSize)
    {
        free(pszTotalInstallSize);
    }
    return dwError;

error:
    goto cleanup;
}
