
function select_tab( this_tab ) {
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
    
	document.getElementById('close_button').style.display = "none";
	document.getElementById('maximize_button').style.display = "none";
	document.getElementById('minimize_button').style.display = "none";
	document.getElementById('programs').className = '';
	document.getElementById('menu').className = 'show';
	document.getElementById('menu').innerHTML = data;
}

function resume_notify( this_obj, program_name ) {
	this_obj = this_obj.parentNode.getElementsByTagName('div')[0];
	if( document.getElementById( "exec." + program_name ) != null )
		this_obj.className = 'notify_resume';
	else
		this_obj.className = 'notify_hide';
}

function show_program( input ) {
	document.getElementById('close_button').style.display = "inline";
	document.getElementById('maximize_button').style.display = "inline";
	document.getElementById('minimize_button').style.display = "inline";
	document.getElementById('menu').className = '';
	document.getElementById('programs').className = 'show';
	if( document.getElementById("exec." + input) == null ) {
		var new_element = document.createElement("div");
		new_element.id = "exec."+input;
		new_element.innerHTML = "<embed src=chrome://plate/content/utils/"+input+" width=100% height=100% />";
		document.getElementById('programs').appendChild(new_element);
	}
	var programs = document.getElementById('programs').getElementsByTagName('div');
	for( var i = 0; i < programs.length; i++ )
		if( programs[i].className == 'show' ) {
			programs[i].className = '';
			break;
		}
	document.getElementById("exec."+input).className = 'show';
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
}

function unmaximize_program() {
	document.getElementById('panel').className = '';
	document.getElementById('content').className = '';
}

function toggle_maximize_program() {
	if( document.getElementById('panel').className == 'maximized' )
		unmaximize_program();
	else
		maximize_program();
}

function $()
{
  var elements = new Array();
  for (var i = 0; i < arguments.length; i++)
  {
    var element = arguments[i];
    if (typeof element == 'string')
      element = document.getElementById(element);
    if (arguments.length == 1)
      return element;
    elements.push(element);
  }
  return elements;
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

function confirm_off() {
	tmp = window.confirm('Do you want to shutdown?'); 
	if (tmp) system('poweroff -f');
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

function update_sysinfo() {

update_div('sysinfo', '/tmp/sysinfo');
setTimeout('update_sysinfo()', 10000);
system('/usr/local/bin/sysinfo');

}

var vis=1;
function cover() {  

	if(vis) {
		vis=0;
		$('cover').style.display='block';
		$('popup').style.display='block';
	} 
	else {   
		vis=1;
		$('cover').style.display='none'; 
		$('popup').style.display='none';
	}

}

function popup(path) {

cover(); 
update_div('popup', '/usr/share/plate/chrome/content/template/'+path+'.html');

}
