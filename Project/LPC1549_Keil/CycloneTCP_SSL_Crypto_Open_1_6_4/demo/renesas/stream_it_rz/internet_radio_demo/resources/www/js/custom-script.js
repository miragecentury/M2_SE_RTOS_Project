$(function() {
	$('#lanSettingsEnableDhcp').bind('change', function(event, ui) {
		if($('#lanSettingsEnableDhcp').val() == 'on')
			$('#lanSettingsManualConfig').hide();
		else
			$('#lanSettingsManualConfig').show();
	});
	
	$('#proxySettingsEnable').bind('change', function(event, ui) {
		if($('#proxySettingsEnable').val() == 'on')
			$('#proxySettings').show();
		else
			$('#proxySettings').hide();
	});
	
	$('.cancelButton').bind('click', function(event, ui) {
		//Revert settings
		getConfig();
	});
	
	$('.saveButton').bind('click', function(event, ui) {
		//Validate forms
		var valid = $('#settingsForm').valid();
		
		//Make sure that the form is valid
		if(valid)
		{
			//Save settings
			setConfig();
		}
		else
		{
			//Settings are not valid
			alert('Invalid settings!');
		}
	});
	
	//16-bit number validator
	$.validator.addMethod('uint16Validator', function(value, element) {
		if(this.optional(element))
			return true;
		
		var n = parseInt(value);
		
		if(isNaN(n))
			return false;
		if(n < 0)
			return false;
		if(n > 65535)
			return false;
		else
			return true;
	}, 'Please enter a valid number.');
	
	//MAC address validator
	$.validator.addMethod('macAddrValidator', function(value, element) {
		return this.optional(element) || /^[0-9A-F][02468ACE]([:-][0-9A-F]{2}){5}$/i.test(value);
	}, 'Please enter a valid MAC address.');
	
	//IPv4 address validator
	$.validator.addMethod('ipv4AddrValidator', function(value, element) {
		return this.optional(element) || /^(25[0-5]|2[0-4]\d|[01]?\d\d?)\.(25[0-5]|2[0-4]\d|[01]?\d\d?)\.(25[0-5]|2[0-4]\d|[01]?\d\d?)\.(25[0-5]|2[0-4]\d|[01]?\d\d?)$/i.test(value);
	}, 'Please enter a valid IPv4 address.');
	
	//IPv6 address validator
	$.validator.addMethod('ipv6AddrValidator', function(value, element) {
		return this.optional(element) || /^((([0-9A-Fa-f]{1,4}:){7}[0-9A-Fa-f]{1,4})|(([0-9A-Fa-f]{1,4}:){6}:[0-9A-Fa-f]{1,4})|(([0-9A-Fa-f]{1,4}:){5}:([0-9A-Fa-f]{1,4}:)?[0-9A-Fa-f]{1,4})|(([0-9A-Fa-f]{1,4}:){4}:([0-9A-Fa-f]{1,4}:){0,2}[0-9A-Fa-f]{1,4})|(([0-9A-Fa-f]{1,4}:){3}:([0-9A-Fa-f]{1,4}:){0,3}[0-9A-Fa-f]{1,4})|(([0-9A-Fa-f]{1,4}:){2}:([0-9A-Fa-f]{1,4}:){0,4}[0-9A-Fa-f]{1,4})|(([0-9A-Fa-f]{1,4}:){6}((\b((25[0-5])|(1\d{2})|(2[0-4]\d)|(\d{1,2}))\b)\.){3}(\b((25[0-5])|(1\d{2})|(2[0-4]\d)|(\d{1,2}))\b))|(([0-9A-Fa-f]{1,4}:){0,5}:((\b((25[0-5])|(1\d{2})|(2[0-4]\d)|(\d{1,2}))\b)\.){3}(\b((25[0-5])|(1\d{2})|(2[0-4]\d)|(\d{1,2}))\b))|(::([0-9A-Fa-f]{1,4}:){0,5}((\b((25[0-5])|(1\d{2})|(2[0-4]\d)|(\d{1,2}))\b)\.){3}(\b((25[0-5])|(1\d{2})|(2[0-4]\d)|(\d{1,2}))\b))|([0-9A-Fa-f]{1,4}::([0-9A-Fa-f]{1,4}:){0,5}[0-9A-Fa-f]{1,4})|(::([0-9A-Fa-f]{1,4}:){0,6}[0-9A-Fa-f]{1,4})|(([0-9A-Fa-f]{1,4}:){1,7}:)|(::))$/i.test(value);
	}, 'Please enter a valid IPv6 address.');
	
	//Hostname validator
	$.validator.addMethod('hostnameValidator', function(value, element) {
		return this.optional(element) || /^(?![0-9]+$)(?!.*-$)(?!-)[a-zA-Z0-9-]{1,63}$/i.test(value);
	}, 'Please enter a valid host name.');
	
	//Server name validator
	$.validator.addMethod('serverNameValidator', function(value, element) {
		if(this.optional(element))
			return true;
		else if(/^(25[0-5]|2[0-4]\d|[01]?\d\d?)\.(25[0-5]|2[0-4]\d|[01]?\d\d?)\.(25[0-5]|2[0-4]\d|[01]?\d\d?)\.(25[0-5]|2[0-4]\d|[01]?\d\d?)$/i.test(value))
			return true;
		else if(/^((([0-9A-Fa-f]{1,4}:){7}[0-9A-Fa-f]{1,4})|(([0-9A-Fa-f]{1,4}:){6}:[0-9A-Fa-f]{1,4})|(([0-9A-Fa-f]{1,4}:){5}:([0-9A-Fa-f]{1,4}:)?[0-9A-Fa-f]{1,4})|(([0-9A-Fa-f]{1,4}:){4}:([0-9A-Fa-f]{1,4}:){0,2}[0-9A-Fa-f]{1,4})|(([0-9A-Fa-f]{1,4}:){3}:([0-9A-Fa-f]{1,4}:){0,3}[0-9A-Fa-f]{1,4})|(([0-9A-Fa-f]{1,4}:){2}:([0-9A-Fa-f]{1,4}:){0,4}[0-9A-Fa-f]{1,4})|(([0-9A-Fa-f]{1,4}:){6}((\b((25[0-5])|(1\d{2})|(2[0-4]\d)|(\d{1,2}))\b)\.){3}(\b((25[0-5])|(1\d{2})|(2[0-4]\d)|(\d{1,2}))\b))|(([0-9A-Fa-f]{1,4}:){0,5}:((\b((25[0-5])|(1\d{2})|(2[0-4]\d)|(\d{1,2}))\b)\.){3}(\b((25[0-5])|(1\d{2})|(2[0-4]\d)|(\d{1,2}))\b))|(::([0-9A-Fa-f]{1,4}:){0,5}((\b((25[0-5])|(1\d{2})|(2[0-4]\d)|(\d{1,2}))\b)\.){3}(\b((25[0-5])|(1\d{2})|(2[0-4]\d)|(\d{1,2}))\b))|([0-9A-Fa-f]{1,4}::([0-9A-Fa-f]{1,4}:){0,5}[0-9A-Fa-f]{1,4})|(::([0-9A-Fa-f]{1,4}:){0,6}[0-9A-Fa-f]{1,4})|(([0-9A-Fa-f]{1,4}:){1,7}:)|(::))$/i.test(value))
			return true;
		else if(/^((?![0-9]+$)(?!.*-$)(?!-)[a-zA-Z0-9-]{1,63}\.)*((?![0-9]+$)(?!.*-$)(?!-)[a-zA-Z0-9-]{1,63})$/i.test(value))
			return true;
		else
			return false;
	}, 'Please enter a valid IP address or a valid host name.');
	
	//URL validator
	$.validator.addMethod('urlValidator', function(value, element) {
		return this.optional(element) || /^[0-9A-Z\-\._/]+$/i.test(value);
	}, 'Please enter a valid URL.');
	
	//Settings form validation
	$('#settingsForm').validate({
		rules: {
			icecastSettingsUrl: {
				required: true,
				urlValidator: true
			},
			icecastSettingsPort: {
				required: true,
				uint16Validator: true
			},
			lanSettingsMacAddr: {
				required: true,
				macAddrValidator: true
			},
			lanSettingsHostName: {
				required: true,
				hostnameValidator: true
			},
			lanSettingsHostAddr: {
				required: true,
				ipv4AddrValidator: true
			},
			lanSettingsSubnetMask: {
				required: true,
				ipv4AddrValidator: true
			},
			lanSettingsDefaultGateway: {
				required: true,
				ipv4AddrValidator: true
			},
			lanSettingsPrimaryDns: {
				required: true,
				ipv4AddrValidator: true
			},
			lanSettingsSecondaryDns: {
				required: true,
				ipv4AddrValidator: true
			},
			proxySettingsName: {
				required: true,
				serverNameValidator: true
			},
			proxySettingsPort: {
				required: true,
				uint16Validator: true
			},
		},
		errorPlacement: function (error, element) {
			error.insertAfter(element.parent());
		},
		errorElement: 'div',
		ignore: ''
	});
	
	//Update page contents
	getConfig();
})

function getConfig() {
	//Perform an AJAX (asynchronous HTTP) request
	$.ajax({
		type: 'GET',
		url: 'get_config.xml',
		data: '',
		cache: false,
		dataType: 'xml',
		success: function(data, textStatus, xhr) {
			//alert('Ajax Status Success: ' + textStatus);
			refreshConfig(data);
		},
		error: function(xhr, textStatus, errorThrown) {
			//alert('Ajax Status Error: ' + textStatus + ' ' + errorThrown);
			refreshConfig(null);
		},
		complete: function(xhr, textStatus, xhr) {
			//alert('Ajax Status Complete: ' + textStatus);
		}
	});
}

function refreshConfig(data) {
	var icecastSettingsUrl = '-';
	var icecastSettingsPort = '-';
	var lanSettingsMacAddr = '-';
	var lanSettingsHostName = '-';
	var lanSettingsEnableDhcp = 0;
	var lanSettingsHostAddr = '-';
	var lanSettingsSubnetMask = '-';
	var lanSettingsDefaultGateway = '-';
	var lanSettingsPrimaryDns = '-';
	var lanSettingsSecondaryDns = '-';
	var proxySettingsEnable = 0;
	var proxySettingsName = '-';
	var proxySettingsPort = '-';
	
	if(data != null) {
		//Parse XML file
		var xmlNode = $('settings', data);
		
		//Retrieve Icecast settings
		icecastSettingsUrl = xmlNode.find('icecast > url').text();
		icecastSettingsPort = xmlNode.find('icecast > port').text();
		
		//Retrieve LAN settings
		lanSettingsMacAddr = xmlNode.find('lan > macAddr').text();
		lanSettingsHostName = xmlNode.find('lan > hostName').text();
		lanSettingsEnableDhcp = parseInt(xmlNode.find('lan > enableDhcp').text());
		lanSettingsHostAddr = xmlNode.find('lan > hostAddr').text();
		lanSettingsSubnetMask = xmlNode.find('lan > subnetMask').text();
		lanSettingsDefaultGateway = xmlNode.find('lan > defaultGateway').text();
		lanSettingsPrimaryDns = xmlNode.find('lan > primaryDns').text();
		lanSettingsSecondaryDns = xmlNode.find('lan > secondaryDns').text();
		
		//Retrieve proxy settings
		proxySettingsEnable = parseInt(xmlNode.find('proxy > enable').text());
		proxySettingsName = xmlNode.find('proxy > name').text();
		proxySettingsPort = xmlNode.find('proxy > port').text();
	}
	
	//Refresh Icecast settings
	$('#icecastSettingsUrl').val(icecastSettingsUrl);
	$('#icecastSettingsPort').val(icecastSettingsPort);
	
	//Refresh LAN settings
	$('#lanSettingsMacAddr').val(lanSettingsMacAddr);
	$('#lanSettingsHostName').val(lanSettingsHostName);
	
	if(lanSettingsEnableDhcp) {
		$('#lanSettingsEnableDhcp').val('on').slider().slider('refresh');
		$('#lanSettingsManualConfig').hide();
	}
	else {
		$('#lanSettingsEnableDhcp').val('off').slider().slider('refresh');
		$('#lanSettingsManualConfig').show();
	}
	
	$('#lanSettingsHostAddr').val(lanSettingsHostAddr);
	$('#lanSettingsSubnetMask').val(lanSettingsSubnetMask);
	$('#lanSettingsDefaultGateway').val(lanSettingsDefaultGateway);
	$('#lanSettingsPrimaryDns').val(lanSettingsPrimaryDns);
	$('#lanSettingsSecondaryDns').val(lanSettingsSecondaryDns);
	
	//Proxy settings
	if(proxySettingsEnable) {
		$('#proxySettingsEnable').val('on').slider().slider('refresh');
		$('#proxySettings').show();
	}
	else {
		$('#proxySettingsEnable').val('off').slider().slider('refresh');
		$('#proxySettings').hide();
	}
	
	$('#proxySettingsName').val(proxySettingsName);
	$('#proxySettingsPort').val(proxySettingsPort);
	
	//Validate form
	$('#settingsForm').valid();
}

function setConfig() {
	//Serialize form
	var form = $('#settingsForm').serialize();
	
	//Perform an AJAX (asynchronous HTTP) request
	$.ajax({
		type: 'POST',
		url: 'set_config.xml',
		data: form,
		cache: false,
		dataType: 'xml',
		success: function(data, textStatus, xhr) {
			//Parse XML file
			var xmlNode = $('data', data);
			//Retrieve status string
			var status = xmlNode.find('status').text();
			//Display status
			alert(status);
		},
		error: function(xhr, textStatus, errorThrown) {
			//Display status
			alert('Connection lost!');
		},
		complete: function(xhr, textStatus, xhr) {
		}
	});
}
