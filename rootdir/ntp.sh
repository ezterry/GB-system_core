#!/system/xbin/sh
# run busybox ntpd

LOGINFO='log -p i -t ntpsetup '

$LOGINFO "Prepare ntpd; checking network connection.."
ipaddr=0.0.0.0

while [ "$ipaddr" == "0.0.0.0" ]
do
    ipaddr=`nslookup pool.ntp.org 2> /dev/null | awk  '/Server/{print $2}'`
    if [ "$ipaddr" == "0.0.0.0" ]
    then
        $LOGINFO "pool.ntp.org not found, no network connection"
        $LOGINFO "Sleep 5min before retry"
        sleep 300
    fi
done
$LOGINFO "DNS lookup successful: pool.ntp.org = $ipaddr"
$LOGINFO "Start ntpd"
exec /system/xbin/ntpd -p pool.ntp.org
