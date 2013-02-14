:: [rom名称]
@del PMFRUS_CHPLUS_RELEASE3.gba
:: [copy 备份rom rom]
@copy pokem.gba PMFRUS_CHPLUS_RELEASE3.gba

:: [[-r rom文件] [-c 码表文件] [-s 导入文本文件名] [-a 直接导入的文本区间文件]]
@import.exe -r PMFRUS_CHPLUS_RELEASE3.gba -c codingList.txt -s text.txt -a offsetAddr.txt
