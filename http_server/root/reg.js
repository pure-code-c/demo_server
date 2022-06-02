var username;
var email;
var pwd;
var pwdOk;
var validate;


function getCookie(name) {
    var strcookie = document.cookie;//获取cookie字符串
    var arrcookie = strcookie.split("; ");//分割
    //遍历匹配
    for (var i = 0; i < arrcookie.length; i++) {
        var arr = arrcookie[i].split("=");
        if (arr[0] == name) {
            return arr[1];
        }
    }
    return "";
}

//获取验证码
function getValidata() {
    var xhttp = new XMLHttpRequest();
    xhttp.onreadystatechange = function () {
        if (this.readyState == 4 && this.status == 200) {
            document.getElementById("validate_jpg").
                setAttribute("src", this.responseText);
        }
    };
    xhttp.open("GET", "/validate?" + Math.random(), true);
    xhttp.send();
}



//发送注册请求
function register() {
    var xhttp = new XMLHttpRequest();
    xhttp.onreadystatechange = function () {
        if (this.readyState == 4 && this.status == 200) {
            alert(this.responseText);
        }
    };
    var str = "/register?{\"username\":\"" + username.value + "\","
        + "\"email\":\"" + email.value + "\"," +
        "\"passwd\":\"" + pwd.value + "\"," +
        "\"code\":\"" + validate.value + "\",";
    xhttp.open("GET", str + "\"xxx\":" + Math.random() + "}", true);
    xhttp.send();
}

//验证用户名
function validate_username(str) {
    var regExp = /^(\w){6,20}$/;
    return regExp.test(str);
}
//验证email
function validate_email(str) {
    var regExp = /^\w+([-+.]\w+)*@\w+([-.]\w+)*\.\w+([-.]\w+)*$/;
    return regExp.test(str);
}
//验证密码
function validate_pwd(str) {
    var regExp = /^[a-zA-Z]\w{5,15}/;
    return regExp.test(str);
}
window.onload = function () {
    getValidata();
    username = document.getElementById("username");
    email = document.getElementById("email");
    pwd = document.getElementById("pwd");
    pwdOk = document.getElementById("pwdOk");
    validate = document.getElementById("validate");

}

//表单提交
function formSubmit() {
    if (!validate_username(username.value))
        alert("用户名错误");
    else if (!validate_email(email.value))
        alert("邮箱错误");
    else if (!validate_pwd(pwd.value))
        alert("密码错误");
    else if (pwd.value != pwdOk.value)
        alert("两次密码不一致");
    else if (validate.value.length != 4)
        alert("验证码错误");
    else
        register();
}