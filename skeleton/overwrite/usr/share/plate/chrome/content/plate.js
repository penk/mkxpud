function initail_plate_ui() {
	select_tab(document.getElementById('home')); 
	system('/usr/local/bin/post-boot.sh');
	update_sysinfo();
	do_i18n();
	init_DBus();
}

if ($.browser.mozilla === true) {
var xpudPrefs = Components.classes["@mozilla.org/preferences-service;1"].getService(Components.interfaces.nsIPrefBranch);
}

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
				//tabs[i].getElementsByTagName('div')[0].className = '';
				break;
			}
		}
		this_tab.className = 'selected';
		//this_tab.getElementsByTagName('div')[0].className = 'under_cover';
		show_menu( this_tab.id );
	}
	else if( document.getElementById('menu').className == '' )
		show_menu( this_tab.id );
}

function show_menu( input ) {

if ($.browser.mozilla === true) {
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
}
	//document.getElementById('close_button').style.display = "none";
	//document.getElementById('maximize_button').style.display = "none";
	//document.getElementById('minimize_button').style.display = "none";
	document.getElementById('top_task').style.display = "none";
	document.getElementById('programs').className = '';
	document.getElementById('menu').className = 'show';

	if ($.browser.mozilla === true) {
		document.getElementById('menu').innerHTML = data;
	} else {
		$("#menu").load("template/" + input + ".html", function() {
		  do_i18n();
		});
	}

	unmaximize_program();
	do_i18n();
}

function show_setting(input) {

	if ($.browser.mozilla === true) {
		show_menu("wrapper"); 
		$('#wrapper').attr('src', "chrome://plate/content/template/" + input + ".html");
	} else {
		$("#menu").load("template/"+input+".html");
		$('#menu').load('template/wrapper.html', function() {
		  $('#wrapper').attr('src', "http://localhost/usr/share/plate/chrome/content/template/" + input + ".html");
		  do_i18n();
		});
	}

}

function resume_notify( this_obj, program_name ) {
	this_obj = this_obj.parentNode.getElementsByTagName('div')[0];
	if( document.getElementById( "exec." + program_name ) != null )
		this_obj.className = 'notify_resume';
	else
		this_obj.className = 'notify_hide';
}

function map_program(xid) {
	console.log('map:' + id + ',' + xid);
	$('#programs').append('<div id="exec.'+id+'" class="show"><embed id="'+xid+'" type="application/x-tableware" width=100% height=100%></embed></div>');
	//$('#programs').addClass('show');
}


// FIXME:  need to use new param here:
// show_program(id, command) 
// for example: show_program('xterm','xterm -bg black -fg gray');

function show_program( input, webapp ) {
	set_page_curl_block();
	
	document.getElementById('top_task').style.display = "inline";
	document.getElementById('menu').className = '';
	document.getElementById('programs').className = 'show';

if ($.browser.mozilla === true) 
{

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
}
else // for webkit-based browser, and using tableware.so plugin
{
		if (webapp == true) { 
				if( document.getElementById("exec." + input) == null )
				$('#programs').append('<div id=exec.'+input+'><iframe src='+input+' width=100% height=100% /></div>');
		}
		else 
		{
			// hide all existing applications
			$('#programs > id').removeClass('show');

			// launch new application
			if( document.getElementById("exec." + id) == null ) 
			{

				console.log('swallow "'+command+'"');
				system('/usr/local/bin/swallow "'+command+'"');

				// FIXME: add loading indicator here

				function update(){
					$.ajax({
						type: "GET",
						url: "http://localhost/cgi-bin/jswrapper?cat%20/tmp/swallow.log",
						success: function(xid){
							console.log(xid);
							map_program(id, xid);
							clearInterval(pollid);
					    	}
					   });
				       };

				var pollid = setInterval(update, 3000);

			} 
			// resume previously opened application 
			else 
			{
				// FIXME: use show/resume class 
				$('#programs').addClass('show');
				$('#exec.'+id).toggleClass('show');
				console.log('resume program: ' + id);

			}
		}
}

/*
	var programs = document.getElementById('programs').getElementsByTagName('div');
	for( var i = 0; i < programs.length; i++ )
		if( programs[i].className == 'show' ) {
			programs[i].className = '';
			break;
		}
	document.getElementById("exec."+input).className = 'show';

	if( document.getElementById('maximize_button').className == 'maximized' )
		maximize_program();
*/
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
	close_page_curl_block();
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

function open_file_tab() {

unmaximize_program();
//document.getElementById('maximize_button').className = '';
show_program("pcmanfm", false);
document.getElementById('top_task').style.display = "none";

}

function set_page_curl_block( html ) {
//$.getJSON("curl.json", function(data){
//	$('#page_curl_block').innerHTML = data.html;
//});

$('body').append('<div id="curl"><img id="curl_img" src="image/curl.png" /><div id="page_curl_block"><div id="close_button" class="w_button" onclick="close_program();"><img src="image/icon_close.png" /> </div><div id="maximize_button" class="w_button" onclick="toggle_maximize_program();"><img src="image/icon_maximize.png" /> </div><div id="minimize_button" class="w_button" onclick="close_page_curl_block(); select_tab(null);"><img src="image/icon_minimize.png" /></div></div></div>');
$('#curl').show();

$("#curl").hover(function(){
    			//create iFrame Shim, append it and.. animate it!
                	$('body').append('<iframe id="curl_shim" frameborder="0" src="javascript:\"\";">');
			if ($.browser.mozilla === true) {
				$('#curl_shim').css('background', 'white'); 
			}
		        $('#curl_shim').stop().animate({
		                width:70,
		                height:70
		        },400);
				$(this).stop().animate({
						width: 135,
						height: 70
				}, 400);
				$('#curl_img').stop().animate({
						width: 135,
						height: 70
					}, 400);			
				$('#page_curl_block').show();
		}, function(){	
				$(this).stop().animate({
					width: 55,
					height: 25
				}, 200);
				$('#curl_img').stop().animate({
						width: 55,
						height: 25
					}, 200);				
		        //Animate iFrame Shim to 0pxx0px
		        $('#curl_shim').stop().animate({
		                width:0,
		                height:0
		        },200);

				$('#page_curl_block').hide();
                		$('#curl_shim').remove();				
});

}

function close_page_curl_block() {
$('#curl').remove();
$('#curl_shim').remove();
}

function system(input) {

	if ($.browser.mozilla === true) {
		netscape.security.PrivilegeManager.enablePrivilege("UniversalXPConnect"); 
		var file = Components.classes["@mozilla.org/file/local;1"].createInstance(Components.interfaces.nsILocalFile);
		file.initWithPath("/usr/local/bin/jswrapper");
		var process = Components.classes["@mozilla.org/process/util;1"].createInstance(Components.interfaces.nsIProcess);
		process.init(file);
		var args = [input];
		process.run(false, args, 1);
	} else {
		$.get('http://localhost/cgi-bin/jswrapper',input);
	}
}

function sleep(milliseconds) {
	var start = new Date().getTime();
	for (var i = 0; i < 1e7; i++) {
		if ((new Date().getTime() - start) > milliseconds) break;
	}
}

function update_div(id, path) {

	if ($.browser.mozilla === true) {
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
	} else {
		$('#'+id).load(path, function() {
			  do_i18n();
		});
	}
}

function update_div_ife(id, path) {

	if ($.browser.mozilla === true) {
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
	} else {
		// FIXME: add timeout
		$('#'+id).load(path, function() {
			  do_i18n();
		});
	}
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
		// close dialog with ESCape key
		$(document).keyup(function(e) {
			if(e.keyCode == 27) {
				$('#cover').hide();
				$('#popup').hide();
				vis=1;
			}
		});
		
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

function get_pref(input) {
	if ($.browser.mozilla === true) {
		var output = xpudPrefs.getCharPref("xpud.locale");
	} else {
		var output = $.ajax({type: "GET", url: "http://localhost/cgi-bin/jswrapper?cat%20/usr/share/plate/defaults/preferences/prefs.js%20|%20grep%20" + input + "%20|%20cut%20-d\\%22%20-f%204", async: false }).responseText;
	}
	output = output.replace(/\n/g, "");
	return output;
}

function set_pref(key, value) {
	if ($.browser.mozilla === true) {
		xpudPrefs.setCharPref(key, value);
	} else {
		$.ajax({type: "GET", url: 'http://localhost/cgi-bin/jswrapper?perl%20-pi%20-e%20%27s/"'+key+'",%20"(.*)"/"'+key+'",%20"'+value+'"/%27%20/usr/share/plate/defaults/preferences/prefs.js', async: false });
	}

}

function do_i18n() {

	var lang = get_pref('xpud.locale');
	$(".i18n").each( function(){
		if(i18n[$(this).text()]) {
			if(i18n[$(this).text()][lang]) {
				$(this).html(i18n[$(this).text()][lang]);
			} else {
				$(this).html(i18n[$(this).text()]["en"]);
			}
		}
	});
}    

function save_preferences() {
	if ($.browser.mozilla === true) {
	var prefService = Components.classes["@mozilla.org/preferences-service;1"]
                               .getService(Components.interfaces.nsIPrefService);
	prefService.savePrefFile(null);
	}
}

function setSelectedOpts(PrefString, ElementName) {
	if ($.browser.mozilla === true) {
	var selVal = xpudPrefs.getCharPref(PrefString);
	var inputs = document.getElementsByName(ElementName); 
	for(var i=0; i < inputs[0].options.length; i++) {      
        if(inputs[0].options[i].value == selVal ) {
                inputs[0].options[i].selected = true;
          }      
     	}
	}
}

var panel_timer; 
var panel_status=0;

function show_panel() {
			$('body').append('<iframe id="panel_shim_1" frameborder="0" src="javascript:\"\";">');

			if (! $('#panel_shim_2').length) {
			$('body').append('<iframe id="panel_shim_2" frameborder="0" src="javascript:\"\";">');
				if ($.browser.mozilla === true) {
					$('#panel_shim_2').css('background', 'white');
				}
			} 

			$('body').append('<div id="closezone"></div>');
			$('#closezone')  
			    .css({  
				position: 'absolute',
				'z-index': '3',
				top: '0px',  
				left: '0px',  
				width: '100%',  
				height: '100%',
				opacity: '0.1',  
				background: '#000'  
			    });  
			$('#closezone').click(function(){ hide_panel(); clearTimeout(panel_timer); });

			$('#panel').animate({"left": "+=110px"}, 150);
			$('#panel').css('-webkit-box-shadow', '10px -2px 12px #818181');
			$('#panel').css('-moz-box-shadow', '10px -2px 12px #818181');
			$('#panel_shim_1').animate({"left": "+=110px"}, 150);
			$('#panel_shim_2').animate({"left": "+=110px"}, 150);
			$('#panel_arrow_img').attr('src', 'image/1leftarrow.png');

			panel_status=1;
}
function hide_panel() {
			$('#panel').animate({"left": "-=110px"}, 150);
			$('#panel_shim_2').animate({"left": "-=110px"}, 150);
			$('#panel_arrow_img').attr('src', 'image/1rightarrow.png');
			$('#panel').css('-webkit-box-shadow', '');
			$('#panel').css('-moz-box-shadow', '');
			$('#closezone').remove();
               		$('#panel_shim_1').remove();
			panel_status=0;
}


$('#closeZone').click(function(){ 
			hide_panel();
});

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
	system('reboot');
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
	system('poweroff');
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
	if ($.browser.mozilla === true) {
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
}

function prefWrite(data, pref) {
	if ($.browser.mozilla === true) {
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
}

function setBackupLocation() {
	prefWrite($('#backup-dir').val(), "backup-dir");
}

//TODO simple template functions:

function setScreenSettings(string) {
	prefWrite(string, "screen-conf");
}

function setLangSettings(string) {
	prefWrite(string, "lang-conf");
}

function setKmapSettings(string) {
	prefWrite(string, "kmap-conf");
}


function loadBackupSettings() {
	if ($.browser.mozilla === true) {
	var restoredFile = '/tmp/restored';
	netscape.security.PrivilegeManager.enablePrivilege("UniversalXPConnect"); 
	var file = Components.classes["@mozilla.org/file/local;1"].createInstance(Components.interfaces.nsILocalFile);
	file.initWithPath(restoredFile);
	if( file.exists()) {   // if it exists we have to check if there's something to restore..
        var kmap = prefRead("kmap-conf");
        var lang = prefRead("lang-conf");
        var screen = prefRead("screen-conf");
        //file.remove(); // Now we've read everything.. remove it!
        system("rm -rf /tmp/restored");
        
        if ( screen != "" ) {
            var values = screen.split('|');
			system('/usr/bin/xrandr --output ' + values[0] + ' --mode ' + values[1] + ' --rotation ' + values[2]);
        }

        if ( kmap != "" ) {
            	system('setxkbmap -layout '+kmap);
            	xpudPrefs.setCharPref("xpud.keymap", kmap);
            	save_preferences();
        }

        if ( lang != "" ) {
        	xpudPrefs.setCharPref("xpud.locale", lang);
        	save_preferences();
        	window.parent.location.reload();        
        }
        system("notify-send 'Restore complete!' -i /usr/share/plate/chrome/content/image/kthememgr.png -u critical");
	}
	}
}	

function init_DBus() {

	if (!window.dbus) { return; } else { console.log('init_DBus()'); } 

	var xbus = dbus.getSignal(dbus.SESSION,'org.xpud.CommInterface', 'CMD', null, '/org/xpud/CommObject', null);
    xbus.onemit = function(args) {

		var input = args.split(':');
		//$('#test').append(args+".<br>");
		// dbus-send --type=signal  /org/xpud/CommObject org.xpud.CommInterface.CMD string:"test"

		switch(input[0]) {

			case "select_tab":
				select_tab(document.getElementById(input[1]));
				break;
			case "show_program": 
				select_tab(document.getElementById('exec'));
				show_program(input[1], false);
				break;
			case "map":
				console.log('map:' + input[1]);
				select_tab(document.getElementById('exec'));

				set_page_curl_block();
	
				document.getElementById('top_task').style.display = "inline";
				document.getElementById('menu').className = '';
				document.getElementById('programs').className = 'show';

				if( document.getElementById("exec." + input[1]) == null ) {
					$('#programs').append('<div id=exec.'+input[1]+' class="show"><embed id="'+input[1]+'" type="application/x-tableware" width=100% height=100%></embed></div>');
				}

				break;
/*
			case "destroy": 
				//$('#test').append("got destroy:"+input[1]+".<br>");
				if ($("#"+input[1]).length != 0) {
					//$('#test').append("remove:"+input[1]+".<br>");
					$("#"+'li.'+input[1]).remove();
					$("#"+'div.'+input[1]).remove();
					$("#"+input[1]).remove();
				} //else { $('#test').append("REMOVE: DOESN'T EXISTS.<br>"); }

				break;
*/
			default:
				console.log(args);
				break;

		}
	
    }

	xbus.enabled = true;
}

