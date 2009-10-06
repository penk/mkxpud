function initail_plate_ui() {
	select_tab(document.getElementById('home')); 
	system('/usr/local/bin/post-boot.sh');
	update_sysinfo();
	do_i18n();
}

var xpudPrefs = Components.classes["@mozilla.org/preferences-service;1"].getService(Components.interfaces.nsIPrefBranch);

function check_enc(num) {
	if (document.getElementById('wifi_connect')) {
	document.getElementById('wifi_connect').disabled = false;
	}
	var enc = document.getElementById('ap-option').options[num].getAttribute('class').toString();
	document.getElementById('enctype').value = enc;
	switch (enc) {
	case 'wifi-wep': // WEP
		document.getElementById('ssid').disabled = true;
		document.getElementById('passwd').disabled = false;
		document.getElementById('ssid-line').style.display = 'none';
		break;
	case 'wifi-wpa': // WPA
		document.getElementById('ssid').disabled = true;
		document.getElementById('passwd').disabled = false;
		document.getElementById('ssid-line').style.display = 'none';
		break;
	case 'wifi-wpa-hidden': // WPA-HIDDEN
		document.getElementById('passwd').disabled = false;
		document.getElementById('ssid').disabled = false;
		document.getElementById('ssid-line').style.display = 'block';
		break;
	case 'wifi-hidden': // hidden node
		document.getElementById('ssid').disabled = true;	
		document.getElementById('passwd').disabled = false;
		document.getElementById('ssid-line').style.display = 'block';
		break;
	default: // none 
		document.getElementById('ssid').disabled = true;		
		document.getElementById('passwd').disabled = true;
		document.getElementById('ssid-line').style.display = 'none';
		break;
	}
}

function send_wifi() {

	var addr = document.getElementById('ap-option').value;
	if (document.getElementById('ssid').value.length != 0) { addr = '"' + document.getElementById('ssid').value + '"'; }
	if (document.getElementById('passwd').value.length == 0) { var passwd = ''; } 
	else { var passwd = '"' + document.getElementById('passwd').value + '"'; }
	var enc = document.getElementById('enctype').value;
	var nic = document.getElementById('nic').value;
//	alert(addr + passwd + enc);
	system('/usr/local/bin/get_wifi ' + nic + ' ' + enc + ' ' + addr + ' ' + passwd);

}
function select_tab( this_tab ) {
	if( this_tab == null ) {
		var tabs = document.getElementById('panel').getElementsByTagName('div');
		for( var i = 0; i < tabs.length; i++ ) {
			if( tabs[i].className == 'selected' )
				this_tab = tabs[i];
		}
	}
	if( this_tab.className == '' ) {
		var tabs = document.getElementById('panel').getElementsByTagName('div');
		for( var i = 0; i < tabs.length; i++ ) {
			if( tabs[i].className == 'selected' ) {
				tabs[i].className = '';
				tabs[i].getElementsByTagName('div')[0].className = '';
				break;
			}
		}
		this_tab.className = 'selected';
		this_tab.getElementsByTagName('div')[0].className = 'under_cover';
		show_menu( this_tab.id );
	}
	else if( document.getElementById('menu').className == '' )
		show_menu( this_tab.id );
}

function show_menu( input ) {
	netscape.security.PrivilegeManager.enablePrivilege("UniversalXPConnect"); 
	var file = Components.classes["@mozilla.org/file/local;1"].createInstance(Components.interfaces.nsILocalFile);
	file.initWithPath("/usr/share/plate/chrome/content/template/" + input + ".html" );

	var fstream = Components.classes["@mozilla.org/network/file-input-stream;1"].createInstance(Components.interfaces.nsIFileInputStream);
	var sstream = Components.classes["@mozilla.org/scriptableinputstream;1"].createInstance(Components.interfaces.nsIScriptableInputStream);
	fstream.init(file, -1, 0, 0);
	sstream.init(fstream); 

	var data = "";
	var str = sstream.read(4096);
	while (str.length > 0) {
		data += str;
		str = sstream.read(4096);
	}

	sstream.close();
	fstream.close();

	var utf8Converter = Components.classes["@mozilla.org/intl/utf8converterservice;1"].getService(Components.interfaces.nsIUTF8ConverterService);
    data = utf8Converter.convertURISpecToUTF8 (data, "UTF-8");
    
	//document.getElementById('close_button').style.display = "none";
	//document.getElementById('maximize_button').style.display = "none";
	//document.getElementById('minimize_button').style.display = "none";
	document.getElementById('top_task').style.display = "none";
	document.getElementById('programs').className = '';
	document.getElementById('menu').className = 'show';
	document.getElementById('menu').innerHTML = data;
	unmaximize_program();
	do_i18n();
}

function resume_notify( this_obj, program_name ) {
	this_obj = this_obj.parentNode.getElementsByTagName('div')[0];
	if( document.getElementById( "exec." + program_name ) != null )
		this_obj.className = 'notify_resume';
	else
		this_obj.className = 'notify_hide';
}

function show_program( input, webapp ) {
	//document.getElementById('close_button').style.display = "inline";
	//document.getElementById('maximize_button').style.display = "inline";
	//document.getElementById('minimize_button').style.display = "inline";
	document.getElementById('top_task').style.display = "inline";
	document.getElementById('menu').className = '';
	document.getElementById('programs').className = 'show';
	if( document.getElementById("exec." + input) == null ) {
		var new_element = document.createElement("div");
		new_element.id = "exec."+input;
		if (webapp == true) { 
			new_element.innerHTML = "<iframe src="+input+" width=100% height=100% />";
		}
		else {
			new_element.innerHTML = "<embed src=chrome://plate/content/utils/app.pud width=100% height=100% command="+input+" />";
		}
		document.getElementById('programs').appendChild(new_element);
	}
	var programs = document.getElementById('programs').getElementsByTagName('div');
	for( var i = 0; i < programs.length; i++ )
		if( programs[i].className == 'show' ) {
			programs[i].className = '';
			break;
		}
	document.getElementById("exec."+input).className = 'show';

	if( document.getElementById('maximize_button').className == 'maximized' )
		maximize_program();
}

function close_program() {
	var programs = document.getElementById('programs').getElementsByTagName('div');
	for( var i = 0; i < programs.length; i++ )
		if( programs[i].className == 'show' )
			document.getElementById('programs').removeChild(programs[i]);
	var tabs = document.getElementById('panel').getElementsByTagName('div');
	for( var i = 0; i < tabs.length; i++ )
		if( tabs[i].className == "selected" )
			show_menu( tabs[i].id );
	unmaximize_program();
}

function maximize_program() {
	document.getElementById('panel').className = 'maximized';
	document.getElementById('content').className = 'maximized';
	document.getElementById('padding').className = 'maximized';
	document.getElementById('programs').className = 'maximized';
	document.getElementById('top_task').className = 'maximized';
	document.getElementById('systray').style.display = 'block';
	top_task_auto_hide();
}

function unmaximize_program() {
	document.getElementById('panel').className = '';
	document.getElementById('content').className = '';
	document.getElementById('padding').className = '';
	document.getElementById('programs').className = '';
	document.getElementById('top_task').className = '';
	document.getElementById('systray').style.display = 'none';
}

function toggle_maximize_program() {
	if( document.getElementById('panel').className == 'maximized' )
	{
		unmaximize_program();
		document.getElementById('maximize_button').className = '';
		document.getElementById('programs').className = 'show';
	}
	else
	{
		maximize_program();
		document.getElementById('maximize_button').className = 'maximized';
	}
}

function system(input) {
	netscape.security.PrivilegeManager.enablePrivilege("UniversalXPConnect"); 
	var file = Components.classes["@mozilla.org/file/local;1"].createInstance(Components.interfaces.nsILocalFile);
	file.initWithPath("/usr/local/bin/jswrapper");
	var process = Components.classes["@mozilla.org/process/util;1"].createInstance(Components.interfaces.nsIProcess);
	process.init(file);
	var args = [input];
	process.run(false, args, 1);
}

function update() {

	sleep(50);

	netscape.security.PrivilegeManager.enablePrivilege("UniversalXPConnect"); 
	var file = Components.classes["@mozilla.org/file/local;1"].createInstance(Components.interfaces.nsILocalFile);
	file.initWithPath("/tmp/jswrapper.log");

	var data = "";
	document.getElementById('info').innerHTML = "";
	var fstream = Components.classes["@mozilla.org/network/file-input-stream;1"].createInstance(Components.interfaces.nsIFileInputStream);
	var sstream = Components.classes["@mozilla.org/scriptableinputstream;1"].createInstance(Components.interfaces.nsIScriptableInputStream);
	fstream.init(file, -1, 0, 0);
	sstream.init(fstream); 

	var str = sstream.read(4096);
	while (str.length > 0) {
		data += str;
		str = sstream.read(4096);
	}

	sstream.close();
	fstream.close();

	document.getElementById('info').innerHTML = data;
}

function sleep(milliseconds) {
	var start = new Date().getTime();
	for (var i = 0; i < 1e7; i++) {
		if ((new Date().getTime() - start) > milliseconds) break;
	}
}

function update_div(id, path) {
	netscape.security.PrivilegeManager.enablePrivilege("UniversalXPConnect"); 
	var file = Components.classes["@mozilla.org/file/local;1"].createInstance(Components.interfaces.nsILocalFile);
	file.initWithPath(path);

	var data = "";
	var fstream = Components.classes["@mozilla.org/network/file-input-stream;1"].createInstance(Components.interfaces.nsIFileInputStream);
	var sstream = Components.classes["@mozilla.org/scriptableinputstream;1"].createInstance(Components.interfaces.nsIScriptableInputStream);
	fstream.init(file, -1, 0, 0);
	sstream.init(fstream); 

	var str = sstream.read(4096);
	while (str.length > 0) {
		data += str;
		str = sstream.read(4096);
	}

	sstream.close();
	fstream.close();

	var utf8Converter = Components.classes["@mozilla.org/intl/utf8converterservice;1"].getService(Components.interfaces.nsIUTF8ConverterService);
    data = utf8Converter.convertURISpecToUTF8 (data, "UTF-8");

	document.getElementById(id).innerHTML = data;
}

function update_div_ife(id, path) {
	netscape.security.PrivilegeManager.enablePrivilege("UniversalXPConnect"); 
	var file = Components.classes["@mozilla.org/file/local;1"].createInstance(Components.interfaces.nsILocalFile);
	file.initWithPath(path);
	// check if file exists
	var i=30; // timeout for 15s
	while ( file.exists() == false && i>0)
	{
		sleep(500);
		i--;
	}
	
	var data = "";
	var fstream = Components.classes["@mozilla.org/network/file-input-stream;1"].createInstance(Components.interfaces.nsIFileInputStream);
	var sstream = Components.classes["@mozilla.org/scriptableinputstream;1"].createInstance(Components.interfaces.nsIScriptableInputStream);
	fstream.init(file, -1, 0, 0);
	sstream.init(fstream); 

	var str = sstream.read(4096);
	while (str.length > 0) {
		data += str;
		str = sstream.read(4096);
	}

	sstream.close();
	fstream.close();

	document.getElementById(id).innerHTML = data;
}

function update_sysinfo() {

if (document.getElementById('sysinfo')) {
update_div('sysinfo', '/tmp/sysinfo');
}

update_div('systray', '/tmp/sysinfo-s');

system('/usr/local/bin/sysinfo');
setTimeout('update_sysinfo()', 3500);
}

var vis=1;
function cover() {

	if(vis) {
		vis=0;
		document.getElementById('cover').style.display='block';
		document.getElementById('popup').style.display='block';
	}
	else {  
		vis=1;
		document.getElementById('cover').style.display='none';
		document.getElementById('popup').style.display='none';
	}
}

function popup(path) {
cover(); 
update_div('popup', '/usr/share/plate/chrome/content/template/'+path+'.html');
do_i18n();
var w = Math.ceil((window.innerWidth - document.getElementById('popup').offsetWidth) / 2);
var h = Math.ceil((window.innerHeight - document.getElementById('popup').offsetHeight) / 2);
document.getElementById('popup').style.left=w+'px';
document.getElementById('popup').style.top=h+'px';
}

function do_i18n() {
	var lang = xpudPrefs.getCharPref("xpud.locale");
	$(".i18n").each(
	   		function(i){
	   			if(i18n[$(this).text()]) {

	   			if(i18n[$(this).text()][lang]) 
	   				$(this).replaceWith(i18n[$(this).text()][lang]);
	   			if((typeof i18n[$(this).text()][lang]  == "undefined") && (i18n[$(this).text()]["en"]))
	   				$(this).replaceWith(i18n[$(this).text()]["en"]);
	   			}
	   			
	   			}
	);
}    

function save_preferences() {
	var prefService = Components.classes["@mozilla.org/preferences-service;1"]
                               .getService(Components.interfaces.nsIPrefService);
	prefService.savePrefFile(null);
}

function setSelectedOpts(PrefString, ElementName) {
	var selVal = xpudPrefs.getCharPref(PrefString);
	var inputs = document.getElementsByName(ElementName); 
	for(var i=0; i < inputs[0].options.length; i++) {      
        if(inputs[0].options[i].value == selVal ) {
                inputs[0].options[i].selected = true;
          }      
     }
}

var top_task_timer;

function top_task_auto_hide() {
	if( document.getElementById('top_task').className == 'maximized' )
	{
		top_task_timer = setTimeout("if(document.getElementById('top_task').className=='maximized')top_task_hide();",1500);
	}
}

function top_task_hide() {
	document.getElementById('top_task').className = 'full_screen';
	document.getElementById('programs').className = 'full_screen';
}

function top_task_show() {
	clearTimeout( top_task_timer );
	if( document.getElementById('top_task').className == 'full_screen' )
	{
		document.getElementById('top_task').className = 'maximized';
		document.getElementById('programs').className = 'maximized';
	}
} 
// FIXME: combine plate_reboot, plate_poweroff and plate_switch_mode functions into one
function plate_reboot() {
	if (document.getElementById('do-backup').checked) { 
		var backupDir = prefRead('backup-dir');
		if (backupDir != '') {
			document.getElementById('shutdown-popup').innerHTML = '<h3>Restart</h3><img src="image/load.gif"> Saving your data...<br />Please wait...';
			system('xpud-backup '+backupDir+' -r');
		}
		else {
			alert('Please set directory for backup');
			cover();
		}
	}
	else {
	system('reboot -f');
	}
}

function plate_poweroff() {
	if (document.getElementById('do-backup').checked) { 
		var backupDir = prefRead('backup-dir');
		if (backupDir != '') {
			document.getElementById('shutdown-popup').innerHTML = '<h3>Poweroff</h3><img src="image/load.gif"> Saving your data...<br />Please wait...';
			system('xpud-backup '+backupDir+' -p');
		}
		else {
			alert('Please set directory for backup');
			cover();
		}
	}
	else {
	system('poweroff -f');
	}
}

function plate_switch_mode() {
	if (document.getElementById('do-backup').checked) { 
		var backupDir = prefRead('backup-dir');
		if (backupDir != '') {
			document.getElementById('shutdown-popup').innerHTML = '<h3>Switch OS</h3><img src="image/load.gif"> Saving your data...<br />Please wait...';
			system('xpud-backup '+backupDir+' -s');
		}
		else {
			alert('Please set directory for backup');
			cover();
		}
	}
	else {
	system('boot-to-win');
	}
}


function prefRead(pref) {
	netscape.security.PrivilegeManager.enablePrivilege("UniversalXPConnect"); 
	var file = Components.classes["@mozilla.org/file/local;1"].createInstance(Components.interfaces.nsILocalFile);
	file.initWithPath('~/.config/plate/'+pref);
	var data = "";
	if( file.exists() ) {
		var fstream = Components.classes["@mozilla.org/network/file-input-stream;1"].createInstance(Components.interfaces.nsIFileInputStream);
		var sstream = Components.classes["@mozilla.org/scriptableinputstream;1"].createInstance(Components.interfaces.nsIScriptableInputStream);
		fstream.init(file, -1, 0, 0);
		sstream.init(fstream); 

		var str = sstream.read(4096);
		while (str.length > 0) {
			data += str;
			str = sstream.read(4096);
		}

		sstream.close();
		fstream.close();

		var utf8Converter = Components.classes["@mozilla.org/intl/utf8converterservice;1"].getService(Components.interfaces.nsIUTF8ConverterService);
		data = utf8Converter.convertURISpecToUTF8 (data, "UTF-8");
	}
	return data;
}

function prefWrite(data, pref) {
	var saveDir = '~/.config/plate';
	var file = Components.classes["@mozilla.org/file/local;1"].createInstance(Components.interfaces.nsILocalFile);
	file.initWithPath(saveDir);
	
	if( !file.exists() || !file.isDirectory() ) {   // if it doesn't exist, create
		file.create(Components.interfaces.nsIFile.DIRECTORY_TYPE, 0777);
	}
	// init file
	file.initWithPath(saveDir+'/'+pref);
	// file is nsIFile, data is a string
	var foStream = Components.classes["@mozilla.org/network/file-output-stream;1"].createInstance(Components.interfaces.nsIFileOutputStream);
	// use 0x02 | 0x10 to open file for appending.
	foStream.init(file, 0x02 | 0x08 | 0x20, 0666, 0); 
	// write, create, truncate
	// In a c file operation, we have no need to set file mode with or operation,
	// directly using "r" or "w" usually.
	// if you are sure there will never ever be any non-ascii text in data you can 
	// also call foStream.writeData directly
	var converter = Components.classes["@mozilla.org/intl/converter-output-stream;1"].createInstance(Components.interfaces.nsIConverterOutputStream);
	converter.init(foStream, "UTF-8", 0, 0);
	converter.writeString(data);
	converter.close(); // this closes foStream
}
