var goodPasswd={
	pruefStaerke: function pruefStaerke(password){

			var staerke = 0; // interner Wert

			if(password.length > 7) {
				staerke += 1;
			}

			if(password.match(/([a-z])/) && password.match(/([A-Z])/)  )
			{
				staerke += 1;
			}

			if(password.match(/([0-9])/) && password.match(/([a-zA-Z])/)) {
				staerke += 1;
			}

			if(password.match(/([!,%,&,@,#,$,^,*,?,_,~])/)){
				staerke += 1;
			}

			if(password.match(/(.*[!,%,&,@,#,$,^,*,?,_,~].*[!,%,&,@,#,$,^,*,?,_,~])/)) {
				staerke += 1;
			}


			if (password.length < 6) {
				//document.getElementById("resultat").removeClass();
				//document.getElementById("resultat").addClass('kurz');
				document.getElementById("resultat").value ="0";
				//return 'Das Passwort ist zu kurz';
			}else if (staerke < 2 ) {
				//document.getElementById("resultat").removeClass();
				//document.getElementById("resultat").addClass('schwach');
				document.getElementById("resultat").value ="25";
				//return 'Das Passwort ist schwach';

			} else if (staerke == 2 ) {
				//document.getElementById("resultat").removeClass();
				//document.getElementById("resultat").addClass('gut');
				document.getElementById("resultat").value ="50";

			} else if (staerke == 3 ) {
				//document.getElementById("resultat").removeClass();
				//document.getElementById("resultat").addClass('stark');
				document.getElementById("resultat").value ="75";
			} else {
				//document.getElementById("resultat").removeClass();
				//document.getElementById("resultat").addClass('stark');
				document.getElementById("resultat").value ="100";
			}
		},

	comparePasswd: function comparePasswd() {

		var StatusIcon=document.getElementById("status-icon");
		// Das Bild hinzufügen wenn es noch nicht da ist
		if(StatusIcon== null){
			document.getElementById("pass_new_2").insertAdjacentHTML('afterend', "<img id='status-icon' class='ok-status-icon' src=''>");
			// Unsichtbar, ohne Quelle
		}

		var a = document.getElementById("pass_new_1");

		var b = document.getElementById("pass_new_2");
		if(a.value.length > 0 && b.value.length > 0){
			//Nur wenn in den Passwortfeldern was drinsteht vergleichen und das Bild sichtbar machen
			StatusIcon.style.display='inline-block';
			if (a.value == b.value) {
				StatusIcon.src='/img/ok.png';
			} else {
				StatusIcon.src='/img/false.png';
			}
		}else{
			//Wenn in den Passwortfeldenr nichtsmehr drinsteht --> unsichtbar machen
			StatusIcon.style.display='none';
		}
	},
}
