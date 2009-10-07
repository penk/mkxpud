/**
 * DBus.js, a JS wrapper for the Browser D-Bus Bridge
 * Copyright (c) 2008-2009  Movial Creative Technologies Inc
 *
 * Contact: Movial Creative Technologies Inc, <info@movial.com>
 * Authors: Lauri Mylläri, <lauri.myllari@movial.com>
 *          Kalle Vahlman, <kalle.vahlman@movial.com>
 *          Ehsun Amanolahi, <ehsun.amanolahi@movial.com>
 *
 * Permission is hereby granted, free of charge, to any person
 * obtaining a copy of this software and associated documentation
 * files (the "Software"), to deal in the Software without
 * restriction, including without limitation the rights to use,
 * copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following
 * conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
 * OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
 * HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 * WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 * OTHER DEALINGS IN THE SOFTWARE.
 */

/**
 * @fileOverview The Browser D-Bus Bridge JavaScript API.
 * @author <a href="mailto:kalle.vahlman@movial.com">Kalle Vahlman</a>
 * @author <a href="mailto:lauri.myllari@movial.com">Lauri Mylläri</a>
 * @author <a href="mailto:ehsun.amanolahi@movial.com">Ehsun Amanolahi</a>
 * @version 1.1
 */

if (!window.DBus)
{
    /**
     * This is the Gecko wrapping for the Browser D-Bus Bridge API
     * You can use the XPCOM API directly, but the Bridge API
     *  - is easier to use
     *  - has few utility features over the "raw" API (like introspection)
     *  - generally feels a bit more JavaScriptish (hopefully)
     */

    /**
     * @private
     * For readability
     */
    var ep = netscape.security.PrivilegeManager.enablePrivilege;
    ep("UniversalXPConnect");
    var cc = Components.classes;
    var ci = Components.interfaces;

    /**
     * @private
     * Utility function that creates a wrapper function which handles signature
     * conversion from wrapper(argarray) to method(arg1, ..., argN) and sets
     * 'this' to point to the user-specified object. This is mainly used to
     * convert the variant arrays from the XPCOM service to callback arguments.
     */
    var unWrap = function(method, user_data) {
        return function() {
            if (method){
                method.apply(user_data, arguments[0]);
            }
        };
    };

    /**
     * @private
     * Returns a function to create an nsIVariant carrying the given value
     * as the specified variant type.
     */
    var makeVariantMaker = function(variant_type) {
        return function(val) {
            ep("UniversalXPConnect");
            var variant = cc["@mozilla.org/variant;1"].createInstance(ci.nsIWritableVariant);
            variant['setAs' + variant_type](val);
            return variant;
        };
        return ret;
    };

    /**
     * @class The Browser D-Bus Bridge API.
     * <h2>About</h2>
     * <p>The Browser D-Bus Bridge is a JavaScript
     * <a href="http://dbus.freedesktop.org">D-Bus</a> bindings implementation
     * for web browsers. The Bridge allows privileged JavaScript code to talk to
     * the D-Bus, both session and system bus (if not prohibited by the D-Bus
     * configuration).
     * <p>Since the implementation differs between web browser
     * engines, the Bridge defines and implements a unified API (referred to as
     * "the Bridge API") to access D-Bus. While you can always use the
     * implementation-specific APIs, their stability is not guaranteed. They
     * also might not support extra features of the Bridge API
     * (like introspection).
     * <h2>Notes about types</h2>
     * <p>The D-Bus specification introduces 16 distinct data types of which 12
     * are "simple" types and 4 are container types. This is a bit of a problem
     * since JavaScript inherenetly only supports five basic types; Boolean,
     * Number, String, Array and Object. There are three ways how the Bridge
     * solves this problem: signatures, autodetection and conversion methods.
     * <p>Every D-Bus message includes a
     * <a href="http://dbus.freedesktop.org/doc/dbus-specification.html
     * #message-protocol-signatures">signature</a> specifying the types of the
     * data it carries. This information has three sources: for signals the
     * received D-Bus message itself, the introspection data of a service and
     * the user of the Bridge API.
     * <p>When the signature is known, the Bridge knows what type the arguments
     * should be and can (try to) automatically convert the data to the right
     * destination type.
     * <p>If there is no signature information available, the Bridge tries to
     * autodetect the signature. Since there are only five basic types in
     * JavaScript, this only works with the following:
     * <ul>
     * <li>JavaScript Boolean to DBUS_TYPE_BOOLEAN
     * <li>JavaScript Number to DBUS_TYPE_DOUBLE
     * <li>JavaScript String to DBUS_TYPE_BOOLEAN
     * <li>JavaScript Array to DBUS_TYPE_ARRAY
     * <li>JavaScript Object to D-Bus DICT type (properties as key-value pairs)
     * </ul>
     * <p>The rest of the types need to be "marked" as specific D-Bus types with
     * the conversion methods. The methods are implementation specific, which
     * means you cannot rely on the return value of the conversion methods. They
     * are meant to only be used when passing arguments to method calls or
     * signals. You will also never be given such object by the Bridge in
     * callbacks, all D-Bus message arguments are converted to one of the five
     * JavaScript types before handing them to the callback.
     * <p>This means that
     * for example the ObjectPath and Signature types are simply strings in the
     * callback. Other exceptions are Structs and Variants. D-Bus Structs are
     * basically just a way to implement a mixed-type array, so they are
     * represented as such in the Bridge API. Variants are a container type that
     * allows any type of content (the type is encoded along with the value),
     * but since JavaScript values have no static typing it is always passed as
     * the contained value to the callbacks.
     * <h2>Getting started</h2>
     * <p>To get started with using the Bridge, continue to the
     * <a href="#constructor">details section of the main class</a>.<br>
     * @description The D-Bus singleton class.
     * <p>The D-Bus Bridge API is based on a singleton class called DBus,
     * through which all operations are made. To get this singleton to your web
     * application, include the <em>dbus.js</em> in the <em>head</em> section:
     * <p><code>&lt;script type='text/javascript' src='dbus.js'>&lt;/script>
     * </code>
     * <p>The DBus singleton is instantiated by <em>dbus.js</em> as a member of
     * the global 'window' object. In other type of applications the
     * <em>dbus.js</em> needs to be loaded and executed in whatever way is
     * available before the Bridge API can be used.
     * <p>The example code in this documentation uses an imaginary
     * <em>Notes</em> service which is specified below. The specification is in
     * <a href="http://dbus.freedesktop.org/doc/dbus-specification.html
     * #introspection-format">the introspection data format</a>
     * of D-Bus, the same type of data which you get by introspecting a service.
     * <p>The preferred method of accessing a D-Bus service is to create an
     * <em>interface object</em> with the <a href="#getInterface">
     * dbus.getInterface()</a> method. It will automatically map an interface
     * of a service that supports the <em>org.freedesktop.DBus.Introspectable
     * </em> interface to a JavaSript object. If the service does not support
     * introspection, you need to use the <a href="#getMethod">dbus.getMethod()
     * </a> and <a href="#getSignal">dbus.getSignal()</a> methods directly to
     * use the service.
     * <pre class="code">
     * <caption>The imaginary Notes interface specification</caption>
     *
     * &lt;?xml version="1.0" encoding="UTF-8"?>
     * &lt;node name="/org/movial/Notes">
     *   &lt;interface name="org.movial.Notes">
     *
     *     &lt;!--
     *       Adds a new note from a dictionary. The key "topic" is a string
     *       to define what the note is about, the rest of the keys are freeform
     *       content shown as a definition list in the note. Returns an ID to
     *       identify the note later.
     *      -->
     *     &lt;method name="AddNote">
     *       &lt;arg name="note_content" type="a{sv}" direction="in" />
     *       &lt;arg name="note_id" type="u" direction="out" />
     *     &lt;/method>
     *
     *     &lt;!-- Removes the note specified by the id. -->
     *     &lt;method name="RemoveNote">
     *       &lt;arg name="note_id" type="u" direction="in" />
     *     &lt;/method>
     *
     *     &lt;!-- Emitted when a note is added -->
     *     &lt;signal name="NoteAdded">
     *       &lt;arg name="note_id" type="u" direction="out" />
     *       &lt;arg name="note_content" type="a{sv}" direction="out" />
     *     &lt;/signal>
     *
     *     &lt;!-- Emitted when a note is removed -->
     *     &lt;signal name="NoteRemoved">
     *       &lt;arg name="note_id" type="u" direction="out" />
     *     &lt;/signal>
     *
     *   &lt;/interface>
     * &lt;/node>
     *
     * </pre>
     */
    DBus = function() {

        ep("UniversalXPConnect");
        this.dbus = cc["@movial.com/dbus/service;1"].getService();
        this.dbus = this.dbus.QueryInterface(Components.interfaces.IDBusService);

        // Mirror static properties
        DBus.prototype.SYSTEM = this.dbus.SYSTEM;
        DBus.prototype.SESSION = this.dbus.SESSION;

    };

    // Methods for non-variant and non-automatic type conversions

    /**
     * Converts the given value to a D-Bus UInt32
     * @function
     * @param val Value to convert
     * @returns An object encapsulating a D-Bus value. It is not possible to
     * convert the return value back to the original value.
     */
    DBus.prototype.UInt32 = makeVariantMaker("Uint32");

    /**
     * Converts the given value to a D-Bus Int32
     * @function
     * @param val Value to convert
     * @returns An object encapsulating a D-Bus value. It is not possible to
     * convert the return value back to the original value.
     */
    DBus.prototype.Int32 = makeVariantMaker("Int32");

    /**
     * Converts the given value to a D-Bus UInt16
     * @function
     * @param val Value to convert
     * @returns An object encapsulating a D-Bus value. It is not possible to
     * convert the return value back to the original value.
     */
    DBus.prototype.UInt16 = makeVariantMaker("Uint16");

    /**
     * Converts the given value to a D-Bus Int16
     * @function
     * @param val Value to convert
     * @returns An object encapsulating a D-Bus value. It is not possible to
     * convert the return value back to the original value.
     */
    DBus.prototype.Int16 = makeVariantMaker("Int16");

    /**
     * Converts the given value to a D-Bus UInt64
     * @function
     * @param val Value to convert
     * @returns An object encapsulating a D-Bus value. It is not possible to
     * convert the return value back to the original value.
     */
    DBus.prototype.UInt64 = makeVariantMaker("Uint64");

    /**
     * Converts the given value to a D-Bus Int64
     * @function
     * @param val Value to convert
     * @returns An object encapsulating a D-Bus value. It is not possible to
     * convert the return value back to the original value.
     */
    DBus.prototype.Int64 = makeVariantMaker("Int64");

    /**
     * Converts the given value to a D-Bus Byte
     * @function
     * @param val Value to convert
     * @returns An object encapsulating a D-Bus value. It is not possible to
     * convert the return value back to the original value.
     */
    DBus.prototype.Byte = makeVariantMaker("Int8");

    /**
     * Converts the given value to a D-Bus ObjectPath
     * @function
     * @param val Value to convert
     * @returns An object encapsulating a D-Bus value. It is not possible to
     * convert the return value back to the original value.
     */
    DBus.prototype.ObjectPath = function(val) {
      ep("UniversalXPConnect");
      var carrier = cc["@movial.com/dbus/datacarrier;1"].createInstance(ci.IDBusDataCarrier);
      carrier.type = "o";
      carrier.value = val;
      return carrier;
    };

    /**
     * Converts the given value to a D-Bus Signature
     * @function
     * @param val Value to convert
     * @returns An object encapsulating a D-Bus value. It is not possible to
     * convert the return value back to the original value.
     */
    DBus.prototype.Signature = function(val) {
      ep("UniversalXPConnect");
      var carrier = cc["@movial.com/dbus/datacarrier;1"].createInstance(ci.IDBusDataCarrier);
      carrier.type = "g";
      carrier.value = val;
      return carrier;
    };

    /**
     * Converts the given value to a D-Bus Variant
     * @function
     * @param val Value to convert
     * @returns An object encapsulating a D-Bus value. It is not possible to
     * convert the return value back to the original value.
     */
    DBus.prototype.Variant = function(sig, val) {
      ep("UniversalXPConnect");
      var carrier = cc["@movial.com/dbus/datacarrier;1"].createInstance(ci.IDBusDataCarrier);
      carrier.type = "v";
      carrier.signature = sig;
      carrier.value = val;
      return carrier;
    };

    /**
     * Converts the given value to a D-Bus Struct
     * @function
     * @param val Value to convert
     * @returns An object encapsulating a D-Bus value. It is not possible to
     * convert the return value back to the original value.
     */
    DBus.prototype.Struct = function(val) {
      ep("UniversalXPConnect");
      var carrier = cc["@movial.com/dbus/datacarrier;1"].createInstance(ci.IDBusDataCarrier);
      carrier.type = "r";
      carrier.value = val;
      return carrier;
    };

    // Lowlevel Bridge API methods

    /**
     *  Acquire a D-Bus Method object.
     *  @description <p>D-Bus method calls are handled as objects in the
     *  Bridge. You can instantiate a method object once and reuse it for
     *  multiple calls of the method. Method objects have two properties to
     *  wich you can set callback functions, <em>onreply</em> and
     *  <em>onerror</em>. The arguments of the callback functions depend
     *  on the interface specification.
     *  <p>The <em>onreply</em> callback is called when the method call has
     *  been made successfully and an reply message has beed received. The
     *  arguments of the callback depend on the interface specification.
     *  <p>The <em>onerror</em> callback is called when either the method
     *  call could not be made (due to wrong parameters or for other reasons)
     *  or the method call has been made, but resulted in an error message to
     *  be received.
     *  <p>By default, the method calls are made asynchronously. If a
     *  synchronous method call is desired (so that the method call waits for
     *  a reply before returning), the <em>async</em> property can be set
     *  to <em>false</em>. As the call depends on reply from a separate
     *  process and can potentially take long to execute, synchronous calls
     *  should be avoided in order to not block the calling process (and thus
     *  possibly cause an unresponsive UI).
     *  @example
     *  <caption>Using the signal objects</caption>
     *  var addNote = dbus.getMethod(dbus.SESSION, 'org.movial.Notes',
     *                                '/org/movial/Notes', 'AddNote');
     *  var noteID = -1;
     *  addNote.onreply = function(id){
     *    // The example reply message has signature of 'u'
     *    noteID = id;
     *  };
     *  addNote.onerror = function(error, message) {
     *    // Error callbacks always have a name as the first argument and
     *    // possibly a descriptive message or other arguments following it.
     *    var msgstr = error;
     *    if (message) {
     *      msgstr += ': ' + message;
     *    }
     *    alert('Adding a note failed!\n' + msgstr);
     *  };
     *  // The signature of this method is 'a{sv}'. Dicts are mapped to
     *  // JavaScript Object class so we construct the argument using the
     *  // object construction notation. Variants are a container type which
     *  // holds any single complete type inside, but in the Bridge API they
     *  // are usually simply represented by their value.
     *  addNote( { "topic": "hot", "amount": 42 } );
     *
     *  (... later ...)
     *
     *  var removeNote = dbus.getMethod(dbus.SESSION, 'org.movial.Notes',
     *                                  '/org/movial/Notes', 'RemoveNote');
     *  // This time we choose not to care if it succeeds or not so we don't
     *  // supply onreply or onerror functions. This allows the actual D-Bus
     *  // message to be sent as "no reply expected" message.
     *  removeNote(noteID);
     *  @function
     *  @param bus dbus.SESSION or dbus.SYSTEM
     *  @param {String} dest destination service
     *  @param {String} path path of the object implementing the interface
     *  @param {String} method_name name of the method to acquire
     *  @param {String} inter interface to acquire
     *  @param {String} signature signature of the method, may be null
     *  @param {Object} data object to use as <em>this</em> in callbacks
     *  @returns {DBus.Method} method object or null on errors
     */
    DBus.prototype.getMethod = function(bustype,
                                        destination,
                                        object_path,
                                        method_name,
                                        inter,
                                        signature,
                                        user_data) {

        /**
         * @private
         * A wrapper function for method calls
         * Executes the actual call through the XPCOM interface
         */
        var method = function() {
            ep("UniversalXPConnect");
            try {
              // The Array.prototype dance converts the "sort of Array but not
              // quite" 'arguments' to a real Array object
              method.dbusmethod.doCall(Array.prototype.slice.call(arguments),
                                       arguments.length);
            } catch (e) {
              if (method._onerror) {
                method._onerror.apply(method.user_data, [e.toString()]);
              }
            }
        };

        // Store the user specified "this" object
        method.user_data = user_data;

        // Create the XPCOM method object
        ep("UniversalXPConnect");
        method.dbusmethod = this.dbus.getMethod(bustype,
                                                destination,
                                                object_path,
                                                method_name,
                                                inter,
                                                signature);

        /* XXX: Hmm? onreply and onerror are undefined here, no? */
        method.dbusmethod.onReply = unWrap(method.onreply, method.user_data);
        method.dbusmethod.onError = unWrap(method.onerror, method.user_data);

        // Watch the onreply property and assign the callback through our
        // wrapper
        method.watch("onreply", function(id, oldval, newval) {
            ep("UniversalXPConnect");
            method.dbusmethod.onReply = unWrap(newval, method.user_data);
        });

        // Ditto for onerror
        method.watch("onerror", function(id, oldval, newval) {
            ep("UniversalXPConnect");
            method.dbusmethod.onError = unWrap(newval, method.user_data);
            // We need to shadow the property for some reason...
            method._onerror = newval;
        });

        // Mirror the async property
        method.async = true;
        method.watch("async", function(id, oldval, newval) {
            ep("UniversalXPConnect");
            method.dbusmethod.async = newval;
        });

        return method;
    };

    /**
     *  Acquire a D-Bus Signal object.
     *  @description <p>D-Bus signals are handled as objects in the Bridge. You
     *  can instantiate a signal object once and reuse it for multiple sessions
     *  of listening to the signal.
     *  <p> The <em>enabled</em> property controls whether the signal is being
     *  listened to or not. In D-Bus terms, setting <em>enabled</em> to true
     *  adds a match rule with the specified arguments and setting it to false
     *  removes it.
     *  <p>The <em>onemit</em> property should be the function to call when
     *  when the signal is emitted on the message bus. The arguments of the
     *  callback depend on the interface specification. You can provide an
     *  object as the last argument for <em>getSignal()</em> which will be
     *  used as the <em>this</em> object in the callback.
     *  @example
     *  <caption>Using the signal objects</caption>
     *  var noteAdded = dbus.getSignal(dbus.SESSION,
     *                                'org.movial.Notes', 'NoteAdded',
     *                                null, null, null);
     *  noteAdded.onemit = function(args) {
     *    // The signature of this signal is 'a{sv}'. 'args' is thus an object
     *    // and we can use the . accessor to get the values (variants are
     *    // always presented by their values in callbacks) since we know the
     *    // hash keys.
     *    alert(args.topic + "\n" + args.amount);
     *  };
     *  noteAdded.enabled = true;
     *
     *  (... later ...)
     *
     *  // We are temporarily not interested in the signal so we disable it
     *  noteAdded.enabled = false;
     *  @function
     *  @param bus dbus.SESSION or dbus.SYSTEM
     *  @param {String} inter interface where the signal is
     *  @param {String} signal_name name of the signal
     *  @param {String} sender sender to filter by, usually null.
     *  @param {String} path object path to filter by, usually null.
     *  @param {Object} data object to use as <em>this</em> in the callback
     *  @returns {DBus.Signal} signal object or null on errors
     */
    DBus.prototype.getSignal = function(bustype,
                                        inter,
                                        signal_name,
                                        sender,
                                        object_path,
                                        user_data) {
        var signal = {};

        // Create the XPCOM signal object
        ep("UniversalXPConnect");
        signal.dbussignal = this.dbus.getSignal(bustype,
                                                inter,
                                                signal_name,
                                                sender,
                                                object_path);

        // Store user-specified "this" for callbacks
        signal.user_data = user_data;

        // XXX: ummm.... onemit == undefined?
        signal.dbussignal.onEmit = unWrap(signal.onemit, signal.user_data);

        // Watch the onemit property for wrapping
        signal.watch("onemit", function(id, oldval, newval) {
            ep("UniversalXPConnect");
            signal.dbussignal.onEmit = unWrap(newval, signal.user_data);
        });

        // Watch the enabled property for wrapping
        signal.enabled = false;
        signal.watch("enabled", function(id, oldval, newval) {
            ep("UniversalXPConnect");
            signal.dbussignal.enabled = newval;
        });

        return signal;
    };

    /**
     *  Emit a signal.
     *  @description <p>Sends a signal message to the D-Bus.
     *  @example
     *  var notifyArgs = { "topic" : "hot", "amount" : 42 };
     *  var nodeID = generateNewIDForNote();
     *  if (!dbus.emitSignal(dbus.SESSION, '/org/movial/Notes',
     *                       'org.movial.Notes', 'NoteAdded',
     *                       'ua{sv}', noteID, notifyArgs)) {
     *    alert("Signal emission failed!");
     *  };
     *  @function
     *  @param bus dbus.SESSION or dbus.SYSTEM
     *  @param {String} path object path that is sending the signal
     *  @param {String} inter interface where the signal is
     *  @param {String} signal_name name of the signal
     *  @param {String} signature signature of the signal, may be null
     *  @param arg1 first argument for the signal
     *  @param ...
     *  @param argN last argument for the signal
     *  @returns Boolean indicating if the emission was done succesfully
     */
    DBus.prototype.emitSignal = function() {
        var args = Array.prototype.splice.call(arguments, 5);
        ep("UniversalXPConnect");
        this.dbus.emitSignal(arguments[0], arguments[1],
                             arguments[2], arguments[3],
                             arguments[4], args, args.length);
    };

}

/**
 * @private
 * Parses the interface description XML to a JS object
 */
DBus.prototype.parseInterface = function(doc, bus, dest, path, inter){
    var ret = {};
    var temp = doc.getElementsByTagName("interface");
    //interesting interface is last
    for( var i = temp.length - 1; i >=0; i--){
        if( temp[i].getAttribute("name") != inter ) { continue; }
        //method name
        var methods = temp[i].getElementsByTagName("method");
        for ( var j = 0, jl = methods.length; j < jl; j ++){
            var name = methods[j].getAttribute("name");
            //params
            var args = methods[j].getElementsByTagName("arg");
            var type = [];

            for( var z = 0, zl = args.length; z < zl; z ++){

                if(  args[z].getAttribute("direction") == "in"){
                    type.push(args[z].getAttribute("type") );
                }

            }
            ret[name] = dbus.getMethod(bus, dest, path,
                                       name, inter, type.join(""), ret );
        }

        //signals
        var signals = temp[i].getElementsByTagName("signal");
        for ( var j = 0, jl = signals.length; j < jl; j ++){
            name = signals[j].getAttribute("name");
            ret[name] = dbus.getSignal(bus,
                                       inter, name, null,
                                       this.object_path, ret);
            ret[name].enabled = false;
            ret[name].onemit = null;
        }
        ret.xml = (new XMLSerializer()).serializeToString(doc);
        return ret;
        // break; //interface found
    }
    return null;
};

/**
 *  Acquire an interface object through Introspection.
 *  <p>Uses the <em>Introspect</em> method on the standard
 *  <em>org.freedesktop.DBus.Introspectable</em> interface to map a given
 *  D-Bus interface to a JavaScript object. All methods and signals of that
 *  interface will be available as members of the object. You can refer to the
 *  <a href="#getMethod">getMethod()</a> and
 *  <a href="#getSignal">getSignal()</a> documentation to learn how to use the
 *  method and signal objects. The produced object is provided as <em>this</em>
 *  in the methods and signals.
 *  <p>Note that introspecting is not done asynchronously (to avoid requiring
 *  a callback to retrieve the interface object) so the <em>getInterface()</em>
 *  call will block until the service responds.
 *  @example
 *  <caption>Using the interface objects</caption>
 *  var notes = dbus.getInterface(dbus.SESSION,
 *                                'org.movial.Notes', '/org/movial/Notes',
 *                                'org.movial.Notes');
 *
 *  // Set up listening to the "NoteAdded" signal.
 *  notes.NoteAdded.onemit = function(args) {
 *    // The signature of this signal is 'a{sv}'. 'args' is thus an object
 *    // and we can use the . accessor to get the values (variants are
 *    // always presented by their values in callbacks) since we know the
 *    // hash keys.
 *    alert(args.topic + "\n" + args.amount);
 *  };
 *  notes.NoteAdded.enabled = true;
 *
 *  // Add a note
 *  var noteID = -1;
 *  notes.AddNote.onreply = function(id){
 *   // The example reply message has signature of 'u'
 *    noteID = id;
 *  };
 *  notes.AddNote.onerror = function(error, message) {
 *    // Error callbacks always have a name as the first argument and
 *    // possibly a descriptive message or other arguments following it.
 *    var msgstr = error;
 *    if (message) {
 *      msgstr += ': ' + message;
 *    }
 *    alert('Adding a note failed!\n' + msgstr);
 *  };
 *  // The signature of this method is 'a{sv}'. Dicts are mapped to
 *  // JavaScript Object class so we construct the argument using the
 *  // object construction notation. Variants are a container type which
 *  // holds any single complete type inside, but in the Bridge API they
 *  // are usually simply represented by their value.
 *  notes.AddNote( { "topic": "hot", "amount": 42 } );
 *
 *  (... later ...)
 *
 *  // Remove the note we added earlier
 *  notes.RemoveNote( noteID );
 *  @param bus dbus.SESSION or dbus.SYSTEM
 *  @param {String} dest destination service
 *  @param {String} path object path
 *  @param {String} inter interface to acquire
 *  @returns {DBus.Interface} an object representing a D-Bus interface
 */
DBus.prototype.getInterface = function(bus, dest, path, inter){
    var doc = null;
    var introspect = dbus.getMethod(bus, dest, path,
                                    "Introspect",
                                    "org.freedesktop.DBus.Introspectable" );
    /**
     * @private
     */
    introspect.onreply = function(s){
        doc = s.replace(/^\s*|\s*$/g,"");
    };
    /**
     * @private
     */
    introspect.onerror = function(s){
        doc = null;
    };
    introspect.async = false;
    introspect();

    if (doc === null) {
        return null;
    }

    doc = (new DOMParser()).parseFromString(doc, "text/xml");

    return dbus.parseInterface(doc, bus, dest, path, inter);
};

/**
 * Conversion functions for default conversion types
 * These are offered mainly just for completeness, there is usually no point in
 * actually using them...
 */

/**
 * Converts the given value to a D-Bus Boolean
 * @param val Value to convert
 * @returns An object encapsulating a D-Bus value. It is not possible to
 * convert the return value back to the original value.
 */
DBus.prototype.Boolean = function (val) { return Boolean(val); };

/**
 * Converts the given value to a D-Bus Double (the JS Number type)
 * @param val Value to convert
 * @returns An object encapsulating a D-Bus value. It is not possible to
 * convert the return value back to the original value.
 */
DBus.prototype.Double = function (val) { return Number(val); };

/**
 * Converts the given value to a D-Bus String
 * @param val Value to convert
 * @returns An object encapsulating a D-Bus value. It is not possible to
 * convert the return value back to the original value.
 */
DBus.prototype.String = function (val) { return String(val); };

/**
 * Converts the given value to a D-Bus Array
 * @param val Value to convert
 * @returns An object encapsulating a D-Bus value. It is not possible to
 * convert the return value back to the original value.
 */
DBus.prototype.Array = function (val) { return val; };

/**
 * Converts the given value to a D-Bus Dict
 * @param val Value to convert
 * @returns An object encapsulating a D-Bus value. It is not possible to
 * convert the return value back to the original value.
 */
DBus.prototype.Dict = function (val) { return val; };

// Instantiate the Bridge API singleton
try {
  window.dbus = new DBus();
} catch (e) {
  window.dbus = null;
}
