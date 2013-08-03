/^BSS / {
	MAC = $2
	wifi[MAC]["enc"] = "Open"
}
$1 == "SSID:" {
	wifi[MAC]["SSID"] = $2
}
$1 == "freq:" {
	wifi[MAC]["freq"] = $2
}
$1 == "signal:" {
	wifi[MAC]["sig"] = $2 " " $3
}
$1 == "capability:" {
	wifi[MAC]["type"] = $2
}
$1 == "WPA:" {
	wifi[MAC]["enc"] = "WPA"
}
$1 == "WEP:" {
	wifi[MAC]["enc"] = "WEP"
}
END {
	for (w in wifi) {
		if ( i++ > 0 ) { printf "\n" }
		printf "SSID: %s\nType: %s\nFrequency: %s\nSignal: %s\nEncryption: %s\n"\
		,wifi[w]["SSID"],wifi[w]["type"],wifi[w]["freq"],wifi[w]["sig"],wifi[w]["enc"]
	}
}
