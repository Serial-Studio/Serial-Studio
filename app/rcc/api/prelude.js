// Hand-maintained SDK prelude (JavaScript). Prepended to the generated command
// wrappers by scripts/generate-sdk.py to form SerialStudio.js -- the single,
// auditable definition of every global a Serial Studio script can call.
//
// The host (C++) installs only the bridge QObjects (__ss, __ss_db, ...). This
// file defines the friendly globals on top of them. Each block is guarded so it
// no-ops when its bridge is absent; function-expression form keeps the guards
// reliable in block scope.

// Notification levels (always available).
var Info = 0, Warning = 1, Critical = 2;

// Data tables and dataset value access (bridge: __ss, FrameBuilder).
if (typeof __ss !== 'undefined') {
  tableGet        = function(t, r)    { return __ss.tableGet(t, r); };
  tableSet        = function(t, r, v) { __ss.tableSet(t, r, v); };
  datasetGetRaw   = function(uid)     { return __ss.datasetGetRaw(uid); };
  datasetGetFinal = function(uid)     { return __ss.datasetGetFinal(uid); };
  if (__ss.mqttPublish)
    mqttPublish = function(t, p, q, r) { return __ss.mqttPublish(t, p, q, r); };
}

// Dashboard control (bridge: __ss_db, DashboardApi).
if (typeof __ss_db !== 'undefined') {
  clearPlots                = function()         { return __ss_db.clearPlots(); };
  setPlotPoints             = function(n)        { return __ss_db.setPlotPoints(n); };
  setTerminalVisible        = function(v)        { return __ss_db.setTerminalVisible(v); };
  setNotificationLogVisible = function(v)        { return __ss_db.setNotificationLogVisible(v); };
  setClockVisible           = function(v)        { return __ss_db.setClockVisible(v); };
  setStopwatchVisible       = function(v)        { return __ss_db.setStopwatchVisible(v); };
  setActiveWorkspace        = function(idOrName) { return __ss_db.setActiveWorkspace(idOrName); };
}

// Device output (bridge: __ss_dw, DeviceWriteApi).
if (typeof __ss_dw !== 'undefined')
  deviceWrite = function(data, sourceId) { return __ss_dw.write(data, sourceId); };

// Action firing (bridge: __ss_af, ActionFireApi).
if (typeof __ss_af !== 'undefined')
  actionFire = function(actionId) { return __ss_af.fire(actionId); };

// Notifications (bridge: __nc, NotificationCenter; Pro only). Accepts (title),
// (title, subtitle), or (channel, title, subtitle).
function __ssNotifyArgs(args) {
  if (args.length <= 1)   return ['Dashboard', args[0] || '', ''];
  if (args.length === 2)  return ['Dashboard', args[0] || '', args[1] || ''];
  return [args[0] || '', args[1] || '', args[2] || ''];
}
if (typeof __nc !== 'undefined') {
  notify = function(level) {
    var a = __ssNotifyArgs(Array.prototype.slice.call(arguments, 1));
    __nc.post(level, a[0], a[1], a[2]);
  };
  notifyInfo     = function() { var a = __ssNotifyArgs(arguments); __nc.postInfo(a[0], a[1], a[2]); };
  notifyWarning  = function() { var a = __ssNotifyArgs(arguments); __nc.postWarning(a[0], a[1], a[2]); };
  notifyCritical = function() { var a = __ssNotifyArgs(arguments); __nc.postCritical(a[0], a[1], a[2]); };
  notifyClear    = function() { var a = __ssNotifyArgs(arguments); __nc.resolve(a[0], a[1], a[2]); };
} else {
  var __ssNoPro = function() {
    throw new Error('notify() requires a Pro license. See https://serial-studio.com/pricing');
  };
  notify = notifyInfo = notifyWarning = notifyCritical = notifyClear = __ssNoPro;
}

// Protocol encoders (pure helpers, always available).
function modbusWriteRegister(address, value) {
  var a = address & 0xFFFF, v = Math.round(value) & 0xFFFF;
  return String.fromCharCode((a >> 8) & 0xFF, a & 0xFF, (v >> 8) & 0xFF, v & 0xFF);
}
function modbusWriteCoil(address, on) {
  return modbusWriteRegister(address, on ? 0xFF00 : 0x0000);
}
function modbusWriteFloat(address, value) {
  var buf = new ArrayBuffer(4);
  new DataView(buf).setFloat32(0, value, false);
  var b = new Uint8Array(buf), a = address & 0xFFFF;
  return String.fromCharCode((a >> 8) & 0xFF, a & 0xFF, b[0], b[1], b[2], b[3]);
}
function canSendFrame(id, payload) {
  var canId = id & 0xFFFF, data = '';
  if (typeof payload === 'string')
    data = payload;
  else if (Array.isArray(payload))
    for (var i = 0; i < payload.length; i++)
      data += String.fromCharCode(payload[i] & 0xFF);
  return String.fromCharCode((canId >> 8) & 0xFF, canId & 0xFF, data.length) + data;
}
function canSendValue(id, value, bytes) {
  bytes = bytes || 2;
  var arr = [], v = Math.round(value);
  for (var i = bytes - 1; i >= 0; i--)
    arr.push((v >> (i * 8)) & 0xFF);
  return canSendFrame(id, arr);
}
