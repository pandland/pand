use anyhow::Error;
use swc_fast_ts_strip::{Options, TransformOutput};
use swc_common::{errors::ColorConfig, sync::Lrc, SourceMap, GLOBALS};

#[no_mangle]
pub extern "C" fn transform_sync(ts_code: *const u8, code_len: usize) -> *mut i8 {
    let ts_code = unsafe { std::slice::from_raw_parts(ts_code, code_len) };
    let input = std::str::from_utf8(ts_code).expect("Invalid UTF-8");

    let options = Default::default();

    let output = GLOBALS
        .set(&Default::default(), || operate(input.to_string(), options))
        .expect("Failed to process file");

    let js_code = output.code;
    let js_code = js_code.into_bytes();

    let result = std::ffi::CString::new(js_code).expect("CString::new failed");
    result.into_raw()
}

fn operate(input: String, options: Options) -> Result<TransformOutput, Error> {
    let cm = Lrc::new(SourceMap::default());

    swc::try_with_handler(
        cm.clone(),
        swc::HandlerOpts {
            color: ColorConfig::Never,
            skip_filename: true,
        },
        |handler| swc_fast_ts_strip::operate(&cm, handler, input, options),
    )
}
