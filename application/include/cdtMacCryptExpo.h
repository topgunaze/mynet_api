/**************************************************************
 * 文件名称:  tfMacCryptExpo.h
 * 作    者:  tonson tang
 * 日    期:  2015.05.11
 * 文件描述:  设备MAC加密校验接口
 * 版    本:  V1.00
 * 修改历史:  
 *     <修改者>   <时间>    <版本 >   <描述>
 
 -------------------------------------------------------------
 * tfKeyCheck函数使用说明
 *
 * 输入：devId  CData设备ID号
 * 函数定义  int tfKeyCheck(unsigned int devId, unsigned char mac[6], unsigned char secret[16]);
 * 函数说明  tfKeyCheck()根据参数devId和mac通过加密算法计算出new_secret,与参数secret进行比较,
             new_secret与secret一致表示MAC校验成功,返回1;否则返回0表示MAC校验失败
 * 输入参数  devId    CData设备的ID号
             mac      生产时写入的设备MAC地址
             secret   生产时写入的设备MAC地址对应的16字节长度的密钥
 *   返回值  MAC校验成功返回1;否则返回0,并通过参数secret返回设备重新计算出的new_secret

 *
 * CData产品对应的设备ID号:
 *      Product             Device ID
        W1014MA             0x05010201
        FD1108S             0x01060201
        FD1104S             0x01070201
        FD1104B             0x01070501
        FD1104Y             0x01070801
        FD1508GS            0x04010101
	FD1216S             0x04010101  
**************************************************************/

#ifndef __TF_MAC_SECRET_EXPO_H__
#define __TF_MAC_SECRET_EXPO_H__

/********************************************************************/
/* tfKeyCheck()       16bytes secret                               */
/* INPUT: device id, mac address, secret key                        */
/* OUTPUT: check OK return 1, otherwise return 0;                   */
/* TESTING:                                                         */
/*   device id:  0x01020304                                         */
/*   mac address:  e0:67:b3:01:02:03                                */
/*   secret key: c1 0a 70 27 ce 2b f3 df 82 99 72 4f 46 85 4c cf    */
/********************************************************************/
int tfKeyCheck(unsigned int devId, unsigned char mac[6], unsigned char secret[16]);


/********************************************************************/
/* tfShortKeyCheck()      4bytes secret                            */
/* INPUT: device id, mac address, short secret key                  */
/* OUTPUT: check OK return 1, otherwise return 0                    */
/* TESTING:                                                         */
/*   device id:  0x01020304                                         */
/*   mac address:  e0:67:b3:01:02:03                                */
/*   secret key: c1 0a 70 27 ce 2b f3 df                            */
/********************************************************************/
int tfShortKeyCheck(unsigned int devId, unsigned char mac[6], unsigned char secret[4]);

/********************************************************************/
/* tfMacLicenseCheck()       16bytes secret                        */
/* INPUT: NULL                                                      */
/* OUTPUT: license OK return 0, license Error return 2,             */
/*           otherwise return -1;                                   */
/* FUNCTION:                                                        */
/*      读取NVRAM里烧写的MAC和License进行校验,校验通过返回0;        */
/*      校验不通过返回2;其他错误返回-1.                             */
/********************************************************************/
int tfMacLicenseCheck(void);


/********************************************************************/
/* bootFirmwareSuccessFlag()                                        */
/* INPUT: NULL                                                      */
/* OUTPUT: success return 0, otherwise return -1;                   */
/* FUNCTION:                                                        */
/*      软件正常启动标记                                            */
/*                                                                  */
/********************************************************************/
int bootFirmwareSuccessFlag(void);


/********************************************************************/
/* bootFirmwareFailedFlag()                                         */
/* INPUT: NULL                                                      */
/* OUTPUT: success return 0, otherwise return -1;                   */
/* FUNCTION:                                                        */
/*      软件异常启动标记                                            */
/*                                                                  */
/********************************************************************/
int bootFirmwareFailedFlag(void);


/********************************************************************/
/* tfValidateManuUser()                                            */
/* INPUT: NULL                                                      */
/* OUTPUT: check ok return 1, otherwise return 0;                   */
/* FUNCTION:                                                        */
/*      生产模式用户密码验证接口                                    */
/*                                                                  */
/********************************************************************/
int tfValidateManuUser(char *pPasswd);


#endif /* __TF_MAC_SECRET_EXPO_H__ */
