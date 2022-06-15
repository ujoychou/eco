/**
 * @fileoverview
 * @enhanceable
 * @suppress {messageConventions} JS Compiler reports an error if a variable or
 *     field starts with 'MSG_' and isn't a translatable message.
 * @public
 */
// GENERATED CODE -- DO NOT EDIT!

var jspb = require('google-protobuf');
var goog = jspb;
var global = Function('return this')();

goog.exportSymbol('proto.eco.net.proto.Logging', null, global);
goog.exportSymbol('proto.eco.net.proto.NetworkCount', null, global);
goog.exportSymbol('proto.eco.net.proto.Service', null, global);
goog.exportSymbol('proto.eco.net.proto.Session', null, global);
goog.exportSymbol('proto.eco.net.proto.SessionCount', null, global);
goog.exportSymbol('proto.eco.net.proto.SessionExcept', null, global);

/**
 * Generated by JsPbCodeGenerator.
 * @param {Array=} opt_data Optional initial data array, typically from a
 * server response, or constructed directly in Javascript. The array is used
 * in place and becomes part of the constructed object. It is not cloned.
 * If no data is provided, the constructed object will be empty, but still
 * valid.
 * @extends {jspb.Message}
 * @constructor
 */
proto.eco.net.proto.Session = function(opt_data) {
  jspb.Message.initialize(this, opt_data, 0, -1, null, null);
};
goog.inherits(proto.eco.net.proto.Session, jspb.Message);
if (goog.DEBUG && !COMPILED) {
  proto.eco.net.proto.Session.displayName = 'proto.eco.net.proto.Session';
}


if (jspb.Message.GENERATE_TO_OBJECT) {
/**
 * Creates an object representation of this proto suitable for use in Soy templates.
 * Field names that are reserved in JavaScript and will be renamed to pb_name.
 * To access a reserved field use, foo.pb_<name>, eg, foo.pb_default.
 * For the list of reserved names please see:
 *     com.google.apps.jspb.JsClassTemplate.JS_RESERVED_WORDS.
 * @param {boolean=} opt_includeInstance Whether to include the JSPB instance
 *     for transitional soy proto support: http://goto/soy-param-migration
 * @return {!Object}
 */
proto.eco.net.proto.Session.prototype.toObject = function(opt_includeInstance) {
  return proto.eco.net.proto.Session.toObject(opt_includeInstance, this);
};


/**
 * Static version of the {@see toObject} method.
 * @param {boolean|undefined} includeInstance Whether to include the JSPB
 *     instance for transitional soy proto support:
 *     http://goto/soy-param-migration
 * @param {!proto.eco.net.proto.Session} msg The msg instance to transform.
 * @return {!Object}
 * @suppress {unusedLocalVariables} f is only used for nested messages
 */
proto.eco.net.proto.Session.toObject = function(includeInstance, msg) {
  var f, obj = {
    id: jspb.Message.getFieldWithDefault(msg, 1, 0),
    owner: jspb.Message.getFieldWithDefault(msg, 2, 0),
    user: jspb.Message.getFieldWithDefault(msg, 5, ""),
    lang: jspb.Message.getFieldWithDefault(msg, 6, ""),
    ip: jspb.Message.getFieldWithDefault(msg, 10, ""),
    port: jspb.Message.getFieldWithDefault(msg, 11, 0),
    state: jspb.Message.getFieldWithDefault(msg, 15, 0),
    stampOpen: jspb.Message.getFieldWithDefault(msg, 16, 0),
    stampClose: jspb.Message.getFieldWithDefault(msg, 17, 0)
  };

  if (includeInstance) {
    obj.$jspbMessageInstance = msg;
  }
  return obj;
};
}


/**
 * Deserializes binary data (in protobuf wire format).
 * @param {jspb.ByteSource} bytes The bytes to deserialize.
 * @return {!proto.eco.net.proto.Session}
 */
proto.eco.net.proto.Session.deserializeBinary = function(bytes) {
  var reader = new jspb.BinaryReader(bytes);
  var msg = new proto.eco.net.proto.Session;
  return proto.eco.net.proto.Session.deserializeBinaryFromReader(msg, reader);
};


/**
 * Deserializes binary data (in protobuf wire format) from the
 * given reader into the given message object.
 * @param {!proto.eco.net.proto.Session} msg The message object to deserialize into.
 * @param {!jspb.BinaryReader} reader The BinaryReader to use.
 * @return {!proto.eco.net.proto.Session}
 */
proto.eco.net.proto.Session.deserializeBinaryFromReader = function(msg, reader) {
  while (reader.nextField()) {
    if (reader.isEndGroup()) {
      break;
    }
    var field = reader.getFieldNumber();
    switch (field) {
    case 1:
      var value = /** @type {number} */ (reader.readUint64());
      msg.setId(value);
      break;
    case 2:
      var value = /** @type {number} */ (reader.readUint32());
      msg.setOwner(value);
      break;
    case 5:
      var value = /** @type {string} */ (reader.readString());
      msg.setUser(value);
      break;
    case 6:
      var value = /** @type {string} */ (reader.readString());
      msg.setLang(value);
      break;
    case 10:
      var value = /** @type {string} */ (reader.readString());
      msg.setIp(value);
      break;
    case 11:
      var value = /** @type {number} */ (reader.readUint32());
      msg.setPort(value);
      break;
    case 15:
      var value = /** @type {number} */ (reader.readUint32());
      msg.setState(value);
      break;
    case 16:
      var value = /** @type {number} */ (reader.readUint64());
      msg.setStampOpen(value);
      break;
    case 17:
      var value = /** @type {number} */ (reader.readUint64());
      msg.setStampClose(value);
      break;
    default:
      reader.skipField();
      break;
    }
  }
  return msg;
};


/**
 * Serializes the message to binary data (in protobuf wire format).
 * @return {!Uint8Array}
 */
proto.eco.net.proto.Session.prototype.serializeBinary = function() {
  var writer = new jspb.BinaryWriter();
  proto.eco.net.proto.Session.serializeBinaryToWriter(this, writer);
  return writer.getResultBuffer();
};


/**
 * Serializes the given message to binary data (in protobuf wire
 * format), writing to the given BinaryWriter.
 * @param {!proto.eco.net.proto.Session} message
 * @param {!jspb.BinaryWriter} writer
 * @suppress {unusedLocalVariables} f is only used for nested messages
 */
proto.eco.net.proto.Session.serializeBinaryToWriter = function(message, writer) {
  var f = undefined;
  f = message.getId();
  if (f !== 0) {
    writer.writeUint64(
      1,
      f
    );
  }
  f = message.getOwner();
  if (f !== 0) {
    writer.writeUint32(
      2,
      f
    );
  }
  f = message.getUser();
  if (f.length > 0) {
    writer.writeString(
      5,
      f
    );
  }
  f = message.getLang();
  if (f.length > 0) {
    writer.writeString(
      6,
      f
    );
  }
  f = message.getIp();
  if (f.length > 0) {
    writer.writeString(
      10,
      f
    );
  }
  f = message.getPort();
  if (f !== 0) {
    writer.writeUint32(
      11,
      f
    );
  }
  f = message.getState();
  if (f !== 0) {
    writer.writeUint32(
      15,
      f
    );
  }
  f = message.getStampOpen();
  if (f !== 0) {
    writer.writeUint64(
      16,
      f
    );
  }
  f = message.getStampClose();
  if (f !== 0) {
    writer.writeUint64(
      17,
      f
    );
  }
};


/**
 * optional uint64 id = 1;
 * @return {number}
 */
proto.eco.net.proto.Session.prototype.getId = function() {
  return /** @type {number} */ (jspb.Message.getFieldWithDefault(this, 1, 0));
};


/** @param {number} value */
proto.eco.net.proto.Session.prototype.setId = function(value) {
  jspb.Message.setProto3IntField(this, 1, value);
};


/**
 * optional uint32 owner = 2;
 * @return {number}
 */
proto.eco.net.proto.Session.prototype.getOwner = function() {
  return /** @type {number} */ (jspb.Message.getFieldWithDefault(this, 2, 0));
};


/** @param {number} value */
proto.eco.net.proto.Session.prototype.setOwner = function(value) {
  jspb.Message.setProto3IntField(this, 2, value);
};


/**
 * optional string user = 5;
 * @return {string}
 */
proto.eco.net.proto.Session.prototype.getUser = function() {
  return /** @type {string} */ (jspb.Message.getFieldWithDefault(this, 5, ""));
};


/** @param {string} value */
proto.eco.net.proto.Session.prototype.setUser = function(value) {
  jspb.Message.setProto3StringField(this, 5, value);
};


/**
 * optional string lang = 6;
 * @return {string}
 */
proto.eco.net.proto.Session.prototype.getLang = function() {
  return /** @type {string} */ (jspb.Message.getFieldWithDefault(this, 6, ""));
};


/** @param {string} value */
proto.eco.net.proto.Session.prototype.setLang = function(value) {
  jspb.Message.setProto3StringField(this, 6, value);
};


/**
 * optional string ip = 10;
 * @return {string}
 */
proto.eco.net.proto.Session.prototype.getIp = function() {
  return /** @type {string} */ (jspb.Message.getFieldWithDefault(this, 10, ""));
};


/** @param {string} value */
proto.eco.net.proto.Session.prototype.setIp = function(value) {
  jspb.Message.setProto3StringField(this, 10, value);
};


/**
 * optional uint32 port = 11;
 * @return {number}
 */
proto.eco.net.proto.Session.prototype.getPort = function() {
  return /** @type {number} */ (jspb.Message.getFieldWithDefault(this, 11, 0));
};


/** @param {number} value */
proto.eco.net.proto.Session.prototype.setPort = function(value) {
  jspb.Message.setProto3IntField(this, 11, value);
};


/**
 * optional uint32 state = 15;
 * @return {number}
 */
proto.eco.net.proto.Session.prototype.getState = function() {
  return /** @type {number} */ (jspb.Message.getFieldWithDefault(this, 15, 0));
};


/** @param {number} value */
proto.eco.net.proto.Session.prototype.setState = function(value) {
  jspb.Message.setProto3IntField(this, 15, value);
};


/**
 * optional uint64 stamp_open = 16;
 * @return {number}
 */
proto.eco.net.proto.Session.prototype.getStampOpen = function() {
  return /** @type {number} */ (jspb.Message.getFieldWithDefault(this, 16, 0));
};


/** @param {number} value */
proto.eco.net.proto.Session.prototype.setStampOpen = function(value) {
  jspb.Message.setProto3IntField(this, 16, value);
};


/**
 * optional uint64 stamp_close = 17;
 * @return {number}
 */
proto.eco.net.proto.Session.prototype.getStampClose = function() {
  return /** @type {number} */ (jspb.Message.getFieldWithDefault(this, 17, 0));
};


/** @param {number} value */
proto.eco.net.proto.Session.prototype.setStampClose = function(value) {
  jspb.Message.setProto3IntField(this, 17, value);
};



/**
 * Generated by JsPbCodeGenerator.
 * @param {Array=} opt_data Optional initial data array, typically from a
 * server response, or constructed directly in Javascript. The array is used
 * in place and becomes part of the constructed object. It is not cloned.
 * If no data is provided, the constructed object will be empty, but still
 * valid.
 * @extends {jspb.Message}
 * @constructor
 */
proto.eco.net.proto.SessionCount = function(opt_data) {
  jspb.Message.initialize(this, opt_data, 0, -1, null, null);
};
goog.inherits(proto.eco.net.proto.SessionCount, jspb.Message);
if (goog.DEBUG && !COMPILED) {
  proto.eco.net.proto.SessionCount.displayName = 'proto.eco.net.proto.SessionCount';
}


if (jspb.Message.GENERATE_TO_OBJECT) {
/**
 * Creates an object representation of this proto suitable for use in Soy templates.
 * Field names that are reserved in JavaScript and will be renamed to pb_name.
 * To access a reserved field use, foo.pb_<name>, eg, foo.pb_default.
 * For the list of reserved names please see:
 *     com.google.apps.jspb.JsClassTemplate.JS_RESERVED_WORDS.
 * @param {boolean=} opt_includeInstance Whether to include the JSPB instance
 *     for transitional soy proto support: http://goto/soy-param-migration
 * @return {!Object}
 */
proto.eco.net.proto.SessionCount.prototype.toObject = function(opt_includeInstance) {
  return proto.eco.net.proto.SessionCount.toObject(opt_includeInstance, this);
};


/**
 * Static version of the {@see toObject} method.
 * @param {boolean|undefined} includeInstance Whether to include the JSPB
 *     instance for transitional soy proto support:
 *     http://goto/soy-param-migration
 * @param {!proto.eco.net.proto.SessionCount} msg The msg instance to transform.
 * @return {!Object}
 * @suppress {unusedLocalVariables} f is only used for nested messages
 */
proto.eco.net.proto.SessionCount.toObject = function(includeInstance, msg) {
  var f, obj = {
    stamp: jspb.Message.getFieldWithDefault(msg, 1, 0),
    sessSize: jspb.Message.getFieldWithDefault(msg, 2, 0),
    connSize: jspb.Message.getFieldWithDefault(msg, 3, 0),
    ddosSize: jspb.Message.getFieldWithDefault(msg, 4, 0)
  };

  if (includeInstance) {
    obj.$jspbMessageInstance = msg;
  }
  return obj;
};
}


/**
 * Deserializes binary data (in protobuf wire format).
 * @param {jspb.ByteSource} bytes The bytes to deserialize.
 * @return {!proto.eco.net.proto.SessionCount}
 */
proto.eco.net.proto.SessionCount.deserializeBinary = function(bytes) {
  var reader = new jspb.BinaryReader(bytes);
  var msg = new proto.eco.net.proto.SessionCount;
  return proto.eco.net.proto.SessionCount.deserializeBinaryFromReader(msg, reader);
};


/**
 * Deserializes binary data (in protobuf wire format) from the
 * given reader into the given message object.
 * @param {!proto.eco.net.proto.SessionCount} msg The message object to deserialize into.
 * @param {!jspb.BinaryReader} reader The BinaryReader to use.
 * @return {!proto.eco.net.proto.SessionCount}
 */
proto.eco.net.proto.SessionCount.deserializeBinaryFromReader = function(msg, reader) {
  while (reader.nextField()) {
    if (reader.isEndGroup()) {
      break;
    }
    var field = reader.getFieldNumber();
    switch (field) {
    case 1:
      var value = /** @type {number} */ (reader.readUint64());
      msg.setStamp(value);
      break;
    case 2:
      var value = /** @type {number} */ (reader.readUint32());
      msg.setSessSize(value);
      break;
    case 3:
      var value = /** @type {number} */ (reader.readUint32());
      msg.setConnSize(value);
      break;
    case 4:
      var value = /** @type {number} */ (reader.readUint32());
      msg.setDdosSize(value);
      break;
    default:
      reader.skipField();
      break;
    }
  }
  return msg;
};


/**
 * Serializes the message to binary data (in protobuf wire format).
 * @return {!Uint8Array}
 */
proto.eco.net.proto.SessionCount.prototype.serializeBinary = function() {
  var writer = new jspb.BinaryWriter();
  proto.eco.net.proto.SessionCount.serializeBinaryToWriter(this, writer);
  return writer.getResultBuffer();
};


/**
 * Serializes the given message to binary data (in protobuf wire
 * format), writing to the given BinaryWriter.
 * @param {!proto.eco.net.proto.SessionCount} message
 * @param {!jspb.BinaryWriter} writer
 * @suppress {unusedLocalVariables} f is only used for nested messages
 */
proto.eco.net.proto.SessionCount.serializeBinaryToWriter = function(message, writer) {
  var f = undefined;
  f = message.getStamp();
  if (f !== 0) {
    writer.writeUint64(
      1,
      f
    );
  }
  f = message.getSessSize();
  if (f !== 0) {
    writer.writeUint32(
      2,
      f
    );
  }
  f = message.getConnSize();
  if (f !== 0) {
    writer.writeUint32(
      3,
      f
    );
  }
  f = message.getDdosSize();
  if (f !== 0) {
    writer.writeUint32(
      4,
      f
    );
  }
};


/**
 * optional uint64 stamp = 1;
 * @return {number}
 */
proto.eco.net.proto.SessionCount.prototype.getStamp = function() {
  return /** @type {number} */ (jspb.Message.getFieldWithDefault(this, 1, 0));
};


/** @param {number} value */
proto.eco.net.proto.SessionCount.prototype.setStamp = function(value) {
  jspb.Message.setProto3IntField(this, 1, value);
};


/**
 * optional uint32 sess_size = 2;
 * @return {number}
 */
proto.eco.net.proto.SessionCount.prototype.getSessSize = function() {
  return /** @type {number} */ (jspb.Message.getFieldWithDefault(this, 2, 0));
};


/** @param {number} value */
proto.eco.net.proto.SessionCount.prototype.setSessSize = function(value) {
  jspb.Message.setProto3IntField(this, 2, value);
};


/**
 * optional uint32 conn_size = 3;
 * @return {number}
 */
proto.eco.net.proto.SessionCount.prototype.getConnSize = function() {
  return /** @type {number} */ (jspb.Message.getFieldWithDefault(this, 3, 0));
};


/** @param {number} value */
proto.eco.net.proto.SessionCount.prototype.setConnSize = function(value) {
  jspb.Message.setProto3IntField(this, 3, value);
};


/**
 * optional uint32 ddos_size = 4;
 * @return {number}
 */
proto.eco.net.proto.SessionCount.prototype.getDdosSize = function() {
  return /** @type {number} */ (jspb.Message.getFieldWithDefault(this, 4, 0));
};


/** @param {number} value */
proto.eco.net.proto.SessionCount.prototype.setDdosSize = function(value) {
  jspb.Message.setProto3IntField(this, 4, value);
};



/**
 * Generated by JsPbCodeGenerator.
 * @param {Array=} opt_data Optional initial data array, typically from a
 * server response, or constructed directly in Javascript. The array is used
 * in place and becomes part of the constructed object. It is not cloned.
 * If no data is provided, the constructed object will be empty, but still
 * valid.
 * @extends {jspb.Message}
 * @constructor
 */
proto.eco.net.proto.SessionExcept = function(opt_data) {
  jspb.Message.initialize(this, opt_data, 0, -1, null, null);
};
goog.inherits(proto.eco.net.proto.SessionExcept, jspb.Message);
if (goog.DEBUG && !COMPILED) {
  proto.eco.net.proto.SessionExcept.displayName = 'proto.eco.net.proto.SessionExcept';
}


if (jspb.Message.GENERATE_TO_OBJECT) {
/**
 * Creates an object representation of this proto suitable for use in Soy templates.
 * Field names that are reserved in JavaScript and will be renamed to pb_name.
 * To access a reserved field use, foo.pb_<name>, eg, foo.pb_default.
 * For the list of reserved names please see:
 *     com.google.apps.jspb.JsClassTemplate.JS_RESERVED_WORDS.
 * @param {boolean=} opt_includeInstance Whether to include the JSPB instance
 *     for transitional soy proto support: http://goto/soy-param-migration
 * @return {!Object}
 */
proto.eco.net.proto.SessionExcept.prototype.toObject = function(opt_includeInstance) {
  return proto.eco.net.proto.SessionExcept.toObject(opt_includeInstance, this);
};


/**
 * Static version of the {@see toObject} method.
 * @param {boolean|undefined} includeInstance Whether to include the JSPB
 *     instance for transitional soy proto support:
 *     http://goto/soy-param-migration
 * @param {!proto.eco.net.proto.SessionExcept} msg The msg instance to transform.
 * @return {!Object}
 * @suppress {unusedLocalVariables} f is only used for nested messages
 */
proto.eco.net.proto.SessionExcept.toObject = function(includeInstance, msg) {
  var f, obj = {
    ip: jspb.Message.getFieldWithDefault(msg, 1, ""),
    stamp: jspb.Message.getFieldWithDefault(msg, 2, 0),
    exceptMode: jspb.Message.getFieldWithDefault(msg, 5, 0),
    exceptTotal: jspb.Message.getFieldWithDefault(msg, 6, 0),
    exceptSize: jspb.Message.getFieldWithDefault(msg, 7, 0)
  };

  if (includeInstance) {
    obj.$jspbMessageInstance = msg;
  }
  return obj;
};
}


/**
 * Deserializes binary data (in protobuf wire format).
 * @param {jspb.ByteSource} bytes The bytes to deserialize.
 * @return {!proto.eco.net.proto.SessionExcept}
 */
proto.eco.net.proto.SessionExcept.deserializeBinary = function(bytes) {
  var reader = new jspb.BinaryReader(bytes);
  var msg = new proto.eco.net.proto.SessionExcept;
  return proto.eco.net.proto.SessionExcept.deserializeBinaryFromReader(msg, reader);
};


/**
 * Deserializes binary data (in protobuf wire format) from the
 * given reader into the given message object.
 * @param {!proto.eco.net.proto.SessionExcept} msg The message object to deserialize into.
 * @param {!jspb.BinaryReader} reader The BinaryReader to use.
 * @return {!proto.eco.net.proto.SessionExcept}
 */
proto.eco.net.proto.SessionExcept.deserializeBinaryFromReader = function(msg, reader) {
  while (reader.nextField()) {
    if (reader.isEndGroup()) {
      break;
    }
    var field = reader.getFieldNumber();
    switch (field) {
    case 1:
      var value = /** @type {string} */ (reader.readString());
      msg.setIp(value);
      break;
    case 2:
      var value = /** @type {number} */ (reader.readUint64());
      msg.setStamp(value);
      break;
    case 5:
      var value = /** @type {number} */ (reader.readUint32());
      msg.setExceptMode(value);
      break;
    case 6:
      var value = /** @type {number} */ (reader.readUint32());
      msg.setExceptTotal(value);
      break;
    case 7:
      var value = /** @type {number} */ (reader.readUint32());
      msg.setExceptSize(value);
      break;
    default:
      reader.skipField();
      break;
    }
  }
  return msg;
};


/**
 * Serializes the message to binary data (in protobuf wire format).
 * @return {!Uint8Array}
 */
proto.eco.net.proto.SessionExcept.prototype.serializeBinary = function() {
  var writer = new jspb.BinaryWriter();
  proto.eco.net.proto.SessionExcept.serializeBinaryToWriter(this, writer);
  return writer.getResultBuffer();
};


/**
 * Serializes the given message to binary data (in protobuf wire
 * format), writing to the given BinaryWriter.
 * @param {!proto.eco.net.proto.SessionExcept} message
 * @param {!jspb.BinaryWriter} writer
 * @suppress {unusedLocalVariables} f is only used for nested messages
 */
proto.eco.net.proto.SessionExcept.serializeBinaryToWriter = function(message, writer) {
  var f = undefined;
  f = message.getIp();
  if (f.length > 0) {
    writer.writeString(
      1,
      f
    );
  }
  f = message.getStamp();
  if (f !== 0) {
    writer.writeUint64(
      2,
      f
    );
  }
  f = message.getExceptMode();
  if (f !== 0) {
    writer.writeUint32(
      5,
      f
    );
  }
  f = message.getExceptTotal();
  if (f !== 0) {
    writer.writeUint32(
      6,
      f
    );
  }
  f = message.getExceptSize();
  if (f !== 0) {
    writer.writeUint32(
      7,
      f
    );
  }
};


/**
 * optional string ip = 1;
 * @return {string}
 */
proto.eco.net.proto.SessionExcept.prototype.getIp = function() {
  return /** @type {string} */ (jspb.Message.getFieldWithDefault(this, 1, ""));
};


/** @param {string} value */
proto.eco.net.proto.SessionExcept.prototype.setIp = function(value) {
  jspb.Message.setProto3StringField(this, 1, value);
};


/**
 * optional uint64 stamp = 2;
 * @return {number}
 */
proto.eco.net.proto.SessionExcept.prototype.getStamp = function() {
  return /** @type {number} */ (jspb.Message.getFieldWithDefault(this, 2, 0));
};


/** @param {number} value */
proto.eco.net.proto.SessionExcept.prototype.setStamp = function(value) {
  jspb.Message.setProto3IntField(this, 2, value);
};


/**
 * optional uint32 except_mode = 5;
 * @return {number}
 */
proto.eco.net.proto.SessionExcept.prototype.getExceptMode = function() {
  return /** @type {number} */ (jspb.Message.getFieldWithDefault(this, 5, 0));
};


/** @param {number} value */
proto.eco.net.proto.SessionExcept.prototype.setExceptMode = function(value) {
  jspb.Message.setProto3IntField(this, 5, value);
};


/**
 * optional uint32 except_total = 6;
 * @return {number}
 */
proto.eco.net.proto.SessionExcept.prototype.getExceptTotal = function() {
  return /** @type {number} */ (jspb.Message.getFieldWithDefault(this, 6, 0));
};


/** @param {number} value */
proto.eco.net.proto.SessionExcept.prototype.setExceptTotal = function(value) {
  jspb.Message.setProto3IntField(this, 6, value);
};


/**
 * optional uint32 except_size = 7;
 * @return {number}
 */
proto.eco.net.proto.SessionExcept.prototype.getExceptSize = function() {
  return /** @type {number} */ (jspb.Message.getFieldWithDefault(this, 7, 0));
};


/** @param {number} value */
proto.eco.net.proto.SessionExcept.prototype.setExceptSize = function(value) {
  jspb.Message.setProto3IntField(this, 7, value);
};



/**
 * Generated by JsPbCodeGenerator.
 * @param {Array=} opt_data Optional initial data array, typically from a
 * server response, or constructed directly in Javascript. The array is used
 * in place and becomes part of the constructed object. It is not cloned.
 * If no data is provided, the constructed object will be empty, but still
 * valid.
 * @extends {jspb.Message}
 * @constructor
 */
proto.eco.net.proto.NetworkCount = function(opt_data) {
  jspb.Message.initialize(this, opt_data, 0, -1, null, null);
};
goog.inherits(proto.eco.net.proto.NetworkCount, jspb.Message);
if (goog.DEBUG && !COMPILED) {
  proto.eco.net.proto.NetworkCount.displayName = 'proto.eco.net.proto.NetworkCount';
}


if (jspb.Message.GENERATE_TO_OBJECT) {
/**
 * Creates an object representation of this proto suitable for use in Soy templates.
 * Field names that are reserved in JavaScript and will be renamed to pb_name.
 * To access a reserved field use, foo.pb_<name>, eg, foo.pb_default.
 * For the list of reserved names please see:
 *     com.google.apps.jspb.JsClassTemplate.JS_RESERVED_WORDS.
 * @param {boolean=} opt_includeInstance Whether to include the JSPB instance
 *     for transitional soy proto support: http://goto/soy-param-migration
 * @return {!Object}
 */
proto.eco.net.proto.NetworkCount.prototype.toObject = function(opt_includeInstance) {
  return proto.eco.net.proto.NetworkCount.toObject(opt_includeInstance, this);
};


/**
 * Static version of the {@see toObject} method.
 * @param {boolean|undefined} includeInstance Whether to include the JSPB
 *     instance for transitional soy proto support:
 *     http://goto/soy-param-migration
 * @param {!proto.eco.net.proto.NetworkCount} msg The msg instance to transform.
 * @return {!Object}
 * @suppress {unusedLocalVariables} f is only used for nested messages
 */
proto.eco.net.proto.NetworkCount.toObject = function(includeInstance, msg) {
  var f, obj = {
    session: jspb.Message.getFieldWithDefault(msg, 1, 0),
    stamp: jspb.Message.getFieldWithDefault(msg, 2, 0),
    reqSize: jspb.Message.getFieldWithDefault(msg, 10, 0),
    rspSize: jspb.Message.getFieldWithDefault(msg, 11, 0),
    reqKbyte: jspb.Message.getFieldWithDefault(msg, 20, 0),
    rspKbyte: jspb.Message.getFieldWithDefault(msg, 21, 0)
  };

  if (includeInstance) {
    obj.$jspbMessageInstance = msg;
  }
  return obj;
};
}


/**
 * Deserializes binary data (in protobuf wire format).
 * @param {jspb.ByteSource} bytes The bytes to deserialize.
 * @return {!proto.eco.net.proto.NetworkCount}
 */
proto.eco.net.proto.NetworkCount.deserializeBinary = function(bytes) {
  var reader = new jspb.BinaryReader(bytes);
  var msg = new proto.eco.net.proto.NetworkCount;
  return proto.eco.net.proto.NetworkCount.deserializeBinaryFromReader(msg, reader);
};


/**
 * Deserializes binary data (in protobuf wire format) from the
 * given reader into the given message object.
 * @param {!proto.eco.net.proto.NetworkCount} msg The message object to deserialize into.
 * @param {!jspb.BinaryReader} reader The BinaryReader to use.
 * @return {!proto.eco.net.proto.NetworkCount}
 */
proto.eco.net.proto.NetworkCount.deserializeBinaryFromReader = function(msg, reader) {
  while (reader.nextField()) {
    if (reader.isEndGroup()) {
      break;
    }
    var field = reader.getFieldNumber();
    switch (field) {
    case 1:
      var value = /** @type {number} */ (reader.readUint64());
      msg.setSession(value);
      break;
    case 2:
      var value = /** @type {number} */ (reader.readUint64());
      msg.setStamp(value);
      break;
    case 10:
      var value = /** @type {number} */ (reader.readUint32());
      msg.setReqSize(value);
      break;
    case 11:
      var value = /** @type {number} */ (reader.readUint32());
      msg.setRspSize(value);
      break;
    case 20:
      var value = /** @type {number} */ (reader.readUint64());
      msg.setReqKbyte(value);
      break;
    case 21:
      var value = /** @type {number} */ (reader.readUint64());
      msg.setRspKbyte(value);
      break;
    default:
      reader.skipField();
      break;
    }
  }
  return msg;
};


/**
 * Serializes the message to binary data (in protobuf wire format).
 * @return {!Uint8Array}
 */
proto.eco.net.proto.NetworkCount.prototype.serializeBinary = function() {
  var writer = new jspb.BinaryWriter();
  proto.eco.net.proto.NetworkCount.serializeBinaryToWriter(this, writer);
  return writer.getResultBuffer();
};


/**
 * Serializes the given message to binary data (in protobuf wire
 * format), writing to the given BinaryWriter.
 * @param {!proto.eco.net.proto.NetworkCount} message
 * @param {!jspb.BinaryWriter} writer
 * @suppress {unusedLocalVariables} f is only used for nested messages
 */
proto.eco.net.proto.NetworkCount.serializeBinaryToWriter = function(message, writer) {
  var f = undefined;
  f = message.getSession();
  if (f !== 0) {
    writer.writeUint64(
      1,
      f
    );
  }
  f = message.getStamp();
  if (f !== 0) {
    writer.writeUint64(
      2,
      f
    );
  }
  f = message.getReqSize();
  if (f !== 0) {
    writer.writeUint32(
      10,
      f
    );
  }
  f = message.getRspSize();
  if (f !== 0) {
    writer.writeUint32(
      11,
      f
    );
  }
  f = message.getReqKbyte();
  if (f !== 0) {
    writer.writeUint64(
      20,
      f
    );
  }
  f = message.getRspKbyte();
  if (f !== 0) {
    writer.writeUint64(
      21,
      f
    );
  }
};


/**
 * optional uint64 session = 1;
 * @return {number}
 */
proto.eco.net.proto.NetworkCount.prototype.getSession = function() {
  return /** @type {number} */ (jspb.Message.getFieldWithDefault(this, 1, 0));
};


/** @param {number} value */
proto.eco.net.proto.NetworkCount.prototype.setSession = function(value) {
  jspb.Message.setProto3IntField(this, 1, value);
};


/**
 * optional uint64 stamp = 2;
 * @return {number}
 */
proto.eco.net.proto.NetworkCount.prototype.getStamp = function() {
  return /** @type {number} */ (jspb.Message.getFieldWithDefault(this, 2, 0));
};


/** @param {number} value */
proto.eco.net.proto.NetworkCount.prototype.setStamp = function(value) {
  jspb.Message.setProto3IntField(this, 2, value);
};


/**
 * optional uint32 req_size = 10;
 * @return {number}
 */
proto.eco.net.proto.NetworkCount.prototype.getReqSize = function() {
  return /** @type {number} */ (jspb.Message.getFieldWithDefault(this, 10, 0));
};


/** @param {number} value */
proto.eco.net.proto.NetworkCount.prototype.setReqSize = function(value) {
  jspb.Message.setProto3IntField(this, 10, value);
};


/**
 * optional uint32 rsp_size = 11;
 * @return {number}
 */
proto.eco.net.proto.NetworkCount.prototype.getRspSize = function() {
  return /** @type {number} */ (jspb.Message.getFieldWithDefault(this, 11, 0));
};


/** @param {number} value */
proto.eco.net.proto.NetworkCount.prototype.setRspSize = function(value) {
  jspb.Message.setProto3IntField(this, 11, value);
};


/**
 * optional uint64 req_kbyte = 20;
 * @return {number}
 */
proto.eco.net.proto.NetworkCount.prototype.getReqKbyte = function() {
  return /** @type {number} */ (jspb.Message.getFieldWithDefault(this, 20, 0));
};


/** @param {number} value */
proto.eco.net.proto.NetworkCount.prototype.setReqKbyte = function(value) {
  jspb.Message.setProto3IntField(this, 20, value);
};


/**
 * optional uint64 rsp_kbyte = 21;
 * @return {number}
 */
proto.eco.net.proto.NetworkCount.prototype.getRspKbyte = function() {
  return /** @type {number} */ (jspb.Message.getFieldWithDefault(this, 21, 0));
};


/** @param {number} value */
proto.eco.net.proto.NetworkCount.prototype.setRspKbyte = function(value) {
  jspb.Message.setProto3IntField(this, 21, value);
};



/**
 * Generated by JsPbCodeGenerator.
 * @param {Array=} opt_data Optional initial data array, typically from a
 * server response, or constructed directly in Javascript. The array is used
 * in place and becomes part of the constructed object. It is not cloned.
 * If no data is provided, the constructed object will be empty, but still
 * valid.
 * @extends {jspb.Message}
 * @constructor
 */
proto.eco.net.proto.Service = function(opt_data) {
  jspb.Message.initialize(this, opt_data, 0, -1, null, null);
};
goog.inherits(proto.eco.net.proto.Service, jspb.Message);
if (goog.DEBUG && !COMPILED) {
  proto.eco.net.proto.Service.displayName = 'proto.eco.net.proto.Service';
}


if (jspb.Message.GENERATE_TO_OBJECT) {
/**
 * Creates an object representation of this proto suitable for use in Soy templates.
 * Field names that are reserved in JavaScript and will be renamed to pb_name.
 * To access a reserved field use, foo.pb_<name>, eg, foo.pb_default.
 * For the list of reserved names please see:
 *     com.google.apps.jspb.JsClassTemplate.JS_RESERVED_WORDS.
 * @param {boolean=} opt_includeInstance Whether to include the JSPB instance
 *     for transitional soy proto support: http://goto/soy-param-migration
 * @return {!Object}
 */
proto.eco.net.proto.Service.prototype.toObject = function(opt_includeInstance) {
  return proto.eco.net.proto.Service.toObject(opt_includeInstance, this);
};


/**
 * Static version of the {@see toObject} method.
 * @param {boolean|undefined} includeInstance Whether to include the JSPB
 *     instance for transitional soy proto support:
 *     http://goto/soy-param-migration
 * @param {!proto.eco.net.proto.Service} msg The msg instance to transform.
 * @return {!Object}
 * @suppress {unusedLocalVariables} f is only used for nested messages
 */
proto.eco.net.proto.Service.toObject = function(includeInstance, msg) {
  var f, obj = {
    stamp: jspb.Message.getFieldWithDefault(msg, 1, 0)
  };

  if (includeInstance) {
    obj.$jspbMessageInstance = msg;
  }
  return obj;
};
}


/**
 * Deserializes binary data (in protobuf wire format).
 * @param {jspb.ByteSource} bytes The bytes to deserialize.
 * @return {!proto.eco.net.proto.Service}
 */
proto.eco.net.proto.Service.deserializeBinary = function(bytes) {
  var reader = new jspb.BinaryReader(bytes);
  var msg = new proto.eco.net.proto.Service;
  return proto.eco.net.proto.Service.deserializeBinaryFromReader(msg, reader);
};


/**
 * Deserializes binary data (in protobuf wire format) from the
 * given reader into the given message object.
 * @param {!proto.eco.net.proto.Service} msg The message object to deserialize into.
 * @param {!jspb.BinaryReader} reader The BinaryReader to use.
 * @return {!proto.eco.net.proto.Service}
 */
proto.eco.net.proto.Service.deserializeBinaryFromReader = function(msg, reader) {
  while (reader.nextField()) {
    if (reader.isEndGroup()) {
      break;
    }
    var field = reader.getFieldNumber();
    switch (field) {
    case 1:
      var value = /** @type {number} */ (reader.readUint64());
      msg.setStamp(value);
      break;
    default:
      reader.skipField();
      break;
    }
  }
  return msg;
};


/**
 * Serializes the message to binary data (in protobuf wire format).
 * @return {!Uint8Array}
 */
proto.eco.net.proto.Service.prototype.serializeBinary = function() {
  var writer = new jspb.BinaryWriter();
  proto.eco.net.proto.Service.serializeBinaryToWriter(this, writer);
  return writer.getResultBuffer();
};


/**
 * Serializes the given message to binary data (in protobuf wire
 * format), writing to the given BinaryWriter.
 * @param {!proto.eco.net.proto.Service} message
 * @param {!jspb.BinaryWriter} writer
 * @suppress {unusedLocalVariables} f is only used for nested messages
 */
proto.eco.net.proto.Service.serializeBinaryToWriter = function(message, writer) {
  var f = undefined;
  f = message.getStamp();
  if (f !== 0) {
    writer.writeUint64(
      1,
      f
    );
  }
};


/**
 * optional uint64 stamp = 1;
 * @return {number}
 */
proto.eco.net.proto.Service.prototype.getStamp = function() {
  return /** @type {number} */ (jspb.Message.getFieldWithDefault(this, 1, 0));
};


/** @param {number} value */
proto.eco.net.proto.Service.prototype.setStamp = function(value) {
  jspb.Message.setProto3IntField(this, 1, value);
};



/**
 * Generated by JsPbCodeGenerator.
 * @param {Array=} opt_data Optional initial data array, typically from a
 * server response, or constructed directly in Javascript. The array is used
 * in place and becomes part of the constructed object. It is not cloned.
 * If no data is provided, the constructed object will be empty, but still
 * valid.
 * @extends {jspb.Message}
 * @constructor
 */
proto.eco.net.proto.Logging = function(opt_data) {
  jspb.Message.initialize(this, opt_data, 0, -1, null, null);
};
goog.inherits(proto.eco.net.proto.Logging, jspb.Message);
if (goog.DEBUG && !COMPILED) {
  proto.eco.net.proto.Logging.displayName = 'proto.eco.net.proto.Logging';
}


if (jspb.Message.GENERATE_TO_OBJECT) {
/**
 * Creates an object representation of this proto suitable for use in Soy templates.
 * Field names that are reserved in JavaScript and will be renamed to pb_name.
 * To access a reserved field use, foo.pb_<name>, eg, foo.pb_default.
 * For the list of reserved names please see:
 *     com.google.apps.jspb.JsClassTemplate.JS_RESERVED_WORDS.
 * @param {boolean=} opt_includeInstance Whether to include the JSPB instance
 *     for transitional soy proto support: http://goto/soy-param-migration
 * @return {!Object}
 */
proto.eco.net.proto.Logging.prototype.toObject = function(opt_includeInstance) {
  return proto.eco.net.proto.Logging.toObject(opt_includeInstance, this);
};


/**
 * Static version of the {@see toObject} method.
 * @param {boolean|undefined} includeInstance Whether to include the JSPB
 *     instance for transitional soy proto support:
 *     http://goto/soy-param-migration
 * @param {!proto.eco.net.proto.Logging} msg The msg instance to transform.
 * @return {!Object}
 * @suppress {unusedLocalVariables} f is only used for nested messages
 */
proto.eco.net.proto.Logging.toObject = function(includeInstance, msg) {
  var f, obj = {

  };

  if (includeInstance) {
    obj.$jspbMessageInstance = msg;
  }
  return obj;
};
}


/**
 * Deserializes binary data (in protobuf wire format).
 * @param {jspb.ByteSource} bytes The bytes to deserialize.
 * @return {!proto.eco.net.proto.Logging}
 */
proto.eco.net.proto.Logging.deserializeBinary = function(bytes) {
  var reader = new jspb.BinaryReader(bytes);
  var msg = new proto.eco.net.proto.Logging;
  return proto.eco.net.proto.Logging.deserializeBinaryFromReader(msg, reader);
};


/**
 * Deserializes binary data (in protobuf wire format) from the
 * given reader into the given message object.
 * @param {!proto.eco.net.proto.Logging} msg The message object to deserialize into.
 * @param {!jspb.BinaryReader} reader The BinaryReader to use.
 * @return {!proto.eco.net.proto.Logging}
 */
proto.eco.net.proto.Logging.deserializeBinaryFromReader = function(msg, reader) {
  while (reader.nextField()) {
    if (reader.isEndGroup()) {
      break;
    }
    var field = reader.getFieldNumber();
    switch (field) {
    default:
      reader.skipField();
      break;
    }
  }
  return msg;
};


/**
 * Serializes the message to binary data (in protobuf wire format).
 * @return {!Uint8Array}
 */
proto.eco.net.proto.Logging.prototype.serializeBinary = function() {
  var writer = new jspb.BinaryWriter();
  proto.eco.net.proto.Logging.serializeBinaryToWriter(this, writer);
  return writer.getResultBuffer();
};


/**
 * Serializes the given message to binary data (in protobuf wire
 * format), writing to the given BinaryWriter.
 * @param {!proto.eco.net.proto.Logging} message
 * @param {!jspb.BinaryWriter} writer
 * @suppress {unusedLocalVariables} f is only used for nested messages
 */
proto.eco.net.proto.Logging.serializeBinaryToWriter = function(message, writer) {
  var f = undefined;
};


goog.object.extend(exports, proto.eco.net.proto);