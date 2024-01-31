function toggleCheckbox(element) {
    var xhr = new XMLHttpRequest();
    if(element.checked)
    { 
        xhr.open("GET", "/update?state=1", true); 
    }
    else 
    { 
        xhr.open("GET", "/update?state=0", true); 
    }
    xhr.send();
}

var buttonFlag = false;
function btn_clicked(element)
{
    var xhr = new XMLHttpRequest();
    xhr.open("GET", "/shutdown", true);
    xhr.send();
}


setInterval(function ( ) {
    var xhttp = new XMLHttpRequest();
    xhttp.onreadystatechange = function() {

    if (this.readyState == 4 && this.status == 200) {
    var inputChecked;
    var outputStateM;

    if( this.responseText == 1){ 
        inputChecked = true;
        outputStateM = "On";
    }
    else { 
        inputChecked = false;
        outputStateM = "Off";
    }
    document.getElementById("output").checked = inputChecked;
    document.getElementById("outputState").innerHTML = outputStateM;
    }
};
xhttp.open("GET", "/state", true);
xhttp.send();
}, 500 ) ;


// setInterval(setToggleElement(), 500);



// setToggleElement()
// {
//     var xhttp = new XMLHttpRequest();
//     xhttp.onreadystatechange = function() {

//     if (this.readyState == 4 && this.status == 200) {
//     var inputChecked;
//     var outputStateM;

//     if( this.responseText == 1){ 
//         inputChecked = true;
//         outputStateM = "On";
//     }
//     else { 
//         inputChecked = false;
//         outputStateM = "Off";
//     }
//     document.getElementById("output").checked = inputChecked;
//     document.getElementById("outputState").innerHTML = outputStateM;
//     }};
//     xhttp.open("GET", "/state", true);
//     xhttp.send();

// }



if (!!window.EventSource)
{
    var source = new EventSource('/events');

    source.addEventListener('open', function(e) {
        console.log("Events Connected");
      }, false);
    
      source.addEventListener('error', function(e) {
        if (e.target.readyState != EventSource.OPEN) {
          console.log("Events Disconnected");
        }
      }, false);
    
      source.addEventListener('message', function(e) {
        console.log("message", e.data);
      }, false);



    source.addEventListener('current_off', function(e){
        // var xhr = new XMLHttpRequest();
        // xhr.open("GET", "/update?state=0", true);
        // xhr.send();
        // var xhttp = new XMLHttpRequest();
        // xhttp.onreadystatechange = function() {

    
        // inputChecked = false;
        // outputStateM = "Off";
        
        // document.getElementById("output").checked = inputChecked;
        // document.getElementById("outputState").innerHTML = outputStateM;
        // };
        // xhttp.open("GET", "/state", true);
        // xhttp.send();
        document.getElementById("output").checked = false;
        document.getElementById("outputState").innerHTML = e.data;
    }, false);

    source.addEventListener('current_on', function(e){
        document.getElementById("output").checked = true;     
        document.getElementById("outputState").innerHTML = e.data;
    }, false);
}
