diagnostic(off, derivative_uniformity);
diagnostic(off, chromium.unreachable_code);
struct FSOut {
  @location(0) sk_FragColor: vec4<f32>,
};
fn _skslMain(_stageOut: ptr<function, FSOut>) {
  {
    loop {
      {
        (*_stageOut).sk_FragColor = vec4<f32>(1.0);
      }
      continuing {
        break if true;
      }
    }
  }
}
@fragment fn main() -> FSOut {
  var _stageOut: FSOut;
  _skslMain(&_stageOut);
  return _stageOut;
}
