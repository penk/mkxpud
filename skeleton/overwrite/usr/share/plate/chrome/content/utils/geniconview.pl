#!/usr/bin/perl

open(F, ">/root/plate/template/iconview.html");

print F <<EOF;
<html>
<head>
<meta http-equiv="Content-Type" content="text/html; charset=big5" />
<script src="/usr/share/plate/protoflow/lib/prototype.js" type="text/javascript"></script>
<script src="/usr/share/plate/protoflow/lib/scriptaculous.js" type="text/javascript"></script>
<script src="/usr/share/plate/protoflow/lib/reflection.js" type="text/javascript"></script>
<script src="/usr/share/plate/protoflow/protoFlow.js" type="text/javascript"></script>
<link rel="stylesheet" href="/usr/share/plate/protoflow/protoFlow.css" type="text/css"/>

<script type="text/javascript" src="../plate.js" ></script>
<script language="Javascript">
	Event.observe(window, 'load', function() {
		cf = new ProtoFlow(\$("protoflow"), {
		captions: true, 
		startIndex: 2,
		useReflection: true,
		enableOnClickScroll: true
		});
	});
</script>
<style>
.status {
    background: rgb(226, 255, 222);
	border: 1px rgb(148,226,164) solid;
	height: 5%;
	width: 120px;
	padding: 3px;
	-moz-border-radius: 12px;
}
body {    font-family: dejavu sans; }
img { border: 0px white solid; }
a
{
    text-decoration: none;
    color: rgb(92, 118, 132);
}

a:hover {
    text-decoration: underline;
}

</style>
</head>
<body>

<div style="float: left; margin-left:0px;margin-right: 0px;"> 
	<a href="#" onclick="system('/root/plate/utils/geniconview.pl $ARGV[0]/../'); sleep(100); window.location.reload();"><div class="status">
Parent Folder
	</div></a>
</div>

<div id="protoflow" style="width: 80%; position: absolute; left: -100px;">

EOF

my @list = `ls -l $ARGV[0]`;

for (@list) {
next if /total/;
chomp($f = (split(/ /,$_))[-1]);
	if (substr($_, 0, 1) eq 'd') {
	printf F qq{<a href="file://$ARGV[0]/%s" ondblclick="system('/root/plate/utils/geniconview.pl $ARGV[0]/$f'); sleep(100); window.location.reload();">
<img src="/root/plate/image/folder.png" alt="%s" /></a>}."\n", $f, $f;
#	} elsif ($f =~/(png|jpg)$/) {
#	printf F qq{<a href="file://$ARGV[0]/%s" ondblclick="window.open(this.href);><img src="file://$ARGV[0]/%s" alt="%s" width="80px" /></a>}."\n", $f, $f, $f;
	} else {
	printf F qq{<a href="file://$ARGV[0]/%s" ondblclick="window.open(this.href);"><img src="/root/plate/image/binary.png" alt="%s"/></a>}."\n", $f, $f;
}
}

print F "</body></html>\n";
close(F);
