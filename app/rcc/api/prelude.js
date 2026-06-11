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
} else if (typeof __ss_control !== 'undefined' && typeof __ss_bridge !== 'undefined') {
  // Control scripts run on a worker thread with no direct __ss bridge; route the
  // table API through the marshalled apiCall so reads/writes happen on the GUI
  // thread. tableGet returns undefined when the table or register does not exist
  // (so `tableGet(t, r) || fallback` keeps working).
  tableGet = function(t, r) {
    var res = __ss_bridge.call('project.dataTable.getValue', { table: t, name: r });
    return (res.ok && res.result) ? res.result.value : undefined;
  };
  tableSet = function(t, r, v) {
    __ss_bridge.call('project.dataTable.setValue', { table: t, name: r, value: v });
  };
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
} else if (typeof __ss_control !== 'undefined' && typeof __ss_bridge !== 'undefined') {
  // Control scripts run on a worker thread with no direct __nc bridge; route notify*
  // through the marshalled apiCall so posting happens on the GUI thread. On non-Pro
  // builds the command is absent and the call returns an error result instead.
  notify = function(level) {
    var a = __ssNotifyArgs(Array.prototype.slice.call(arguments, 1));
    return __ss_bridge.call('notifications.post',
                            { level: level, channel: a[0], title: a[1], subtitle: a[2] });
  };
  notifyInfo = function() {
    var a = __ssNotifyArgs(arguments);
    return notify(Info, a[0], a[1], a[2]);
  };
  notifyWarning = function() {
    var a = __ssNotifyArgs(arguments);
    return notify(Warning, a[0], a[1], a[2]);
  };
  notifyCritical = function() {
    var a = __ssNotifyArgs(arguments);
    return notify(Critical, a[0], a[1], a[2]);
  };
  notifyClear = function() {
    var a = __ssNotifyArgs(arguments);
    return __ss_bridge.call('notifications.resolve',
                            { channel: a[0], title: a[1], subtitle: a[2] });
  };
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

// Control-script See/Decide/Act helpers (bridge: __ss_bridge, worker thread only).
// newFrame() returns the latest received frame exactly once per arrival (null when
// nothing new); refreshDashboard() re-runs every dataset transform from the last
// received values and republishes the frames to the dashboard (no export side
// effects) -- call it after tableSet() writes so they render while the device is
// silent; ensureDashboard(spec) declaratively creates whatever the spec
// describes that does not exist yet -- groups matched by title, datasets matched
// by parser index inside their group. Existing items are never modified, and the
// last satisfied spec is memoized so calling it every loop() is free.
if (typeof __ss_bridge !== 'undefined') {
  var __ssLastFrameSeq = 0;
  var __ssEnsuredSig   = '';

  newFrame = function(sourceId) {
    var p = {};
    if (typeof sourceId === 'number')
      p.sourceId = sourceId;
    var r = __ss_bridge.call('io.getLatestFrame', p);
    if (!r.ok || !r.result || !r.result.hasData)
      return null;
    if (r.result.sequence === __ssLastFrameSeq)
      return null;
    __ssLastFrameSeq = r.result.sequence;
    return r.result;
  };

  refreshDashboard = function() {
    return __ss_bridge.call('dashboard.reprocess', {});
  };

  var __ssGroupWidgets = {
    datagrid: 0, accelerometer: 1, gyroscope: 2, gps: 3,
    multiplot: 4, none: 5, plot3d: 6, image: 7, painter: 8
  };
  var __ssDatasetBits = {
    plot: 1, fft: 2, bar: 4, gauge: 8, compass: 16, led: 32, waterfall: 64
  };
  var __ssDatasetOptions = function(ds) {
    if (typeof ds.options === 'number')
      return ds.options;
    var bits = 0;
    for (var key in __ssDatasetBits)
      if (ds[key])
        bits |= __ssDatasetBits[key];
    return bits;
  };
  var __ssEnsureFail = function(out, error) {
    out.ok    = false;
    out.error = error || 'ensureDashboard: API call failed';
    return out;
  };

  ensureDashboard = function(spec) {
    var out = { ok: true, createdGroups: 0, createdDatasets: 0 };
    if (!spec || !spec.length)
      return out;

    var sig = JSON.stringify(spec);
    if (sig === __ssEnsuredSig)
      return out;

    var groupsRes = __ss_bridge.call('project.group.list', {});
    var dsRes     = __ss_bridge.call('project.dataset.list', {});
    if (!groupsRes.ok || !dsRes.ok)
      return __ssEnsureFail(out, groupsRes.error || dsRes.error);

    var groups   = groupsRes.result.groups || [];
    var datasets = dsRes.result.datasets || [];

    for (var g = 0; g < spec.length; ++g) {
      var want    = spec[g];
      var groupId = -1;
      for (var i = 0; i < groups.length; ++i)
        if (groups[i].title === want.title) { groupId = groups[i].groupId; break; }

      if (groupId < 0) {
        var widgetType = __ssGroupWidgets[want.widget || 'none'];
        if (typeof widgetType !== 'number')
          widgetType = __ssGroupWidgets.none;
        var added = __ss_bridge.call('project.group.add',
                                     { title: want.title, widgetType: widgetType });
        if (!added.ok)
          return __ssEnsureFail(out, added.error);
        var relist = __ss_bridge.call('project.group.list', {});
        if (!relist.ok)
          return __ssEnsureFail(out, relist.error);
        groups = relist.result.groups || [];
        for (var j = 0; j < groups.length; ++j)
          if (groups[j].title === want.title) { groupId = groups[j].groupId; break; }
        if (groupId < 0)
          return __ssEnsureFail(out, 'ensureDashboard: group missing after add');
        out.createdGroups++;
      }

      var have = {};
      for (var d = 0; d < datasets.length; ++d)
        if (datasets[d].groupId === groupId)
          have[datasets[d].index] = true;

      var wantDs = want.datasets || [];
      for (var k = 0; k < wantDs.length; ++k) {
        var ds = wantDs[k];
        if (have[ds.index])
          continue;

        var addDs = __ss_bridge.call('project.dataset.addMany', {
          groupId: groupId,
          count: 1,
          options: __ssDatasetOptions(ds),
          titlePattern: ds.title || ('Channel ' + ds.index),
          startIndex: ds.index
        });
        if (!addDs.ok)
          return __ssEnsureFail(out, addDs.error);

        var created = ((addDs.result && addDs.result.created) || [])[0];
        if (ds.units && created)
          __ss_bridge.call('project.dataset.update',
                           { groupId: groupId, datasetId: created.datasetId, units: ds.units });
        out.createdDatasets++;
      }
    }

    __ssEnsuredSig = sig;
    return out;
  };
}
