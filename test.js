const assert = require("assert");
const sample = require("bindings")("node-addson-sample.node");

try {
    sample.Add(100);
}
catch(e) {
    assert(e.toString() == "TypeError: Wrong number of arguments");
}

assert(sample.Add(100, 200) == 300);

const fileList = sample.GetFileList();
assert(fileList.length == 3);
assert(fileList[1].filePath == "/root/1.txt");
assert(fileList[1].fileSize == 100);

sample.GetPower10(2, (err, result)=>{
    assert(err == null);
    assert(result == 1024);
});

sample.GetPower10(-2, (err, result)=>{
    assert(err instanceof Error);
    assert(err.toString() == "Error: N must larger than 0");
    assert(result == null);
});

sample.GetPower20(2)
    .then((result)=>{
        assert(result == 1048576);
    })
    .catch((err)=>{
        assert(false);
    });

sample.GetPower20(-2)
    .then((result)=>{
        assert(false);
    })
    .catch((e)=>{
        assert(e instanceof Error);
        assert(e.toString() == "Error: N must larger than 0");
    });

sample.GetPower30(2, (err, result)=>{
    assert(err == null);
    assert(result == 1073741824);
});

sample.GetPower30(-2, (err, result)=>{
    assert(err instanceof Error);
    assert(err.toString() == "Error: N must larger than 0");
    assert(result == null);
});