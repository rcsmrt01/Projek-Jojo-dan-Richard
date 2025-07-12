
const char webPage[] = R"=====(
<!DOCTYPE html>
<html lang="en">

<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>HALAMAN KONTROL</title>
</head>
<style>
    .box {
        border: 1px solid black;
        padding: 10px;
        margin: 10px;
        width: 40px;
        height: 40px;
        text-align: center;
    }

    .container {
        display: flex;
        justify-content: space-between;
    }

    .biru {
        background-color: blue;
    }

    .jam-relay {
        display: flex;
        align-items: center;
        gap: 10px;
    }

    #jamRelayLPemanas,
    #jamRelayLKandang {
        margin: 10;
    }
</style>

<body>
    <h4>Kipas 1</h4>
    <div id="relay1">
        <button id="tombolonrelay1">ON</button>
        <button id="tomboloffrelay1">OFF</button>
    </div>
    <h4>Kipas 2</h4>
    <div id="relay2">
        <button id="tombolonrelay2">ON</button>
        <button id="tomboloffrelay2">OFF</button>
    </div>
    <h4>Kipas 3</h4>
    <div id="relay3">
        <button id="tombolonrelay3">ON</button>
        <button id="tomboloffrelay3">OFF</button>
    </div>
    <h4>Kipas 4 (Kipas Gas Amoniak)</h4>
    <div id="relay4">
        <button id="tombolonrelay4">ON</button>
        <button id="tomboloffrelay4">OFF</button>
    </div>

    <div class='clock'>
        <h1>Timer Lampu Kandang</h1>
        <h1>dan Lampu Pemanas </h1>
        <p class='time'>Time - <span id='_HOUR'>00</span>:<span id='_MIN'>00</span>:<span id='_SEC'>00</span></p>
        <p class='date'>Date - <span id='_DAY'>00</span>/<span id='_MONTH'>00</span>/<span id='_YEAR'>0000</span></p>
    </div>

    <h4>Lampu Pemanas</h4>
    <div id="relay5">
        <button id="tombolonrelay5">ON</button>
        <button id="tomboloffrelay5">OFF</button>
    </div>
    <div id="jamRelayLPemanas" class="jam-relay"></div>
    <input type="number" id="setJamPemanas" placeholder="Jam" min="0" max="23" />
    <input type="number" id="setMenitPemanas" placeholder="Menit" min="0" max="59" />
    <input type="number" id="offJamPemanas" placeholder="Jam" min="0" max="23" />
    <input type="number" id="offMenitPemanas" placeholder="Menit" min="0" max="59" />
    <div id="hariLPemanas">
        <label><input type="checkbox" value="Senin"> Senin</label>
        <label><input type="checkbox" value="Selasa"> Selasa</label>
        <label><input type="checkbox" value="Rabu"> Rabu</label>
        <label><input type="checkbox" value="Kamis"> Kamis</label>
        <label><input type="checkbox" value="Jumat"> Jumat</label>
        <label><input type="checkbox" value="Sabtu"> Sabtu</label>
        <label><input type="checkbox" value="Minggu"> Minggu</label>
    </div>
    <button id="setLPemanas">Set Waktu Pemanas</button>
    <br />
    <small id="infoJamRelayLPemanas">Info: Lampu akan dinyalakan pada pukul - WIB dan akan dimatikan pada
        pukul - WIB</small>
    <h4>Lampu Kandang</h4>
    <div id="relay6">
        <button id="tombolonrelay6">ON</button>
        <button id="tomboloffrelay6">OFF</button>
    </div>
    <div id="jamRelayLKandang" class="jam-relay"></div>
    <input type="number" id="setJamKandang" placeholder="Jam" min="0" max="23" />
    <input type="number" id="setMenitKandang" placeholder="Menit" min="0" max="59" />
    <input type="number" id="offJamKandang" placeholder="Jam" min="0" max="23" />
    <input type="number" id="offMenitKandang" placeholder="Menit" min="0" max="59" />
    <div id="hariLKandang">
        <label><input type="checkbox" value="Senin"> Senin</label>
        <label><input type="checkbox" value="Selasa"> Selasa</label>
        <label><input type="checkbox" value="Rabu"> Rabu</label>
        <label><input type="checkbox" value="Kamis"> Kamis</label>
        <label><input type="checkbox" value="Jumat"> Jumat</label>
        <label><input type="checkbox" value="Sabtu"> Sabtu</label>
        <label><input type="checkbox" value="Minggu"> Minggu</label>
    </div>
    <button id="setLKandang">Set Waktu Lampu Kandang</button>
    <br />
    <small id="infoJamRelayLKandang">Info: Lampu akan dinyalakan pada pukul - WIB dan akan dimatikan pada
        pukul - WIB</small>

    <br />
    <br />
    <h4>Monitoring Suhu dan Kelembapan Kandang</h4>
    <div id="registrasiPemeliharaanAyam">
        <form id="formRegistrasi">
            <label for="registrasiPemeliharaanAyam"> Masukkan Tanggal Start Periode Pemeliharaan: </label>
            <input type="date" id="startDate">
            <br>
            <label for="registrasiPemeliharaanAyam"> Masukkan Tanggal Akhir Pemeliharaan Ayam</label>
            <input type="date" id="endDate" name="endDate" readonly>

            <input type="text" id="formattedStartDate" readonly>
            <input type="text" id="formattedEndDate" readonly>
            <br>
            <input type="submit" id="kirimStartTanggal" value="Submit">
        </form>
    </div>
    <br>
    <div id="formSuhu">
        <form id="suhuForm">
            <label for="isiNilai">Isi Nilai Suhu</label>
            <input type="number" id="isiNilai" min="16" max="45" value="25" /><button id="kirimNilai">Kirimkan Nilai
                Suhu</button>
            <br />
            <small id="tampilanSuhuDikirim">Nilai suhu yang dijaga: </small>
            <br />
            <br />
        </form>
    </div>
    <br />
    <section>
        <h4>Nilai Suhu saat ini:</h4>
        <div class="container box" id="sdrDHT11_1">-</div>
        <br />
        <h4>Nilai Kelembapan saat ini:</h4>
        <div class="container box" id="kdrDHT11_1">-</div>
    </section>
    <br />
    <h4>Monitoring Kadar Gas Amoniak (NH4)</h4>
    <section>
        <h4>Kadar Gas NH4 saat ini: (batas aman kadar gas amoniak = 0.5 ppm)</h4>
        <div class="container box" id="amoMQ137_1">-</div>
    </section>
    <script>

        var socket;
        function init() {
            socket = new WebSocket("ws://" + window.location.hostname + ":81/");
            socket.onopen = function () {
                console.log("Websocket connection estabilished");
            };


            socket.onmessage = function (event) {
                var data = JSON.parse(event.data);
                console.log("Data berhasil diterima dari Websocket:", data);
                //untuk konfirmasi bahwa suhu telah berhasil disimpan dari webpage ke eeprom esp32
                if (data.type === "konfirmasiSuhu") {
                    var konfirmasi = data.value;
                    console.log(konfirmasi);
                }

                //untuk penerimaan nilai suhu dari DHT11
                if (data.type === "suhu1") {
                    var suhu = data.value;
                    document.getElementById("sdrDHT11_1").textContent = suhu + "°C";
                    console.log("Data pembacaan suhu dari DHT11 diterima: " + suhu);
                }

                if (data.type === "kelembapan1") {
                    var kelembapan = data.value;
                    document.getElementById("kdrDHT11_1").textContent = kelembapan + "%";
                    console.log("Data pembacaan kelembapan dari DHT11 diterima: " + kelembapan);
                }

                if (data.type === "Amoniak1") {
                    var nAmoniak = data.value;
                    document.getElementById("amoMQ137_1").textContent = nAmoniak + " ppm";
                    console.log(
                        "Data pembacaan ppm amoniak dari MQ137 diterima: " + nAmoniak);
                }
            }
            socket.onerror = function (error) {
                console.log("Websocket error: ".error);
            };
        }

        document.getElementById("kirimNilai").addEventListener("click", function (event) {
            event.preventDefault();
            uploadSuhu();
        });

        function uploadSuhu() {
            var nilaiSuhu = document.getElementById("isiNilai").value;
            //menampilkan nilai suhu yang dikirim
            document.getElementById("tampilanSuhuDikirim").innerText = "Nilai suhu yang dijaga: " + nilaiSuhu + "°C";
            //kirim nilai suhu dari webpage ke esp32
            if (socket && socket.readyState === WebSocket.OPEN) {
                socket.send(JSON.stringify({ suhu: nilaiSuhu }));
            } else {
                console.log("Suhu gagal dikirim");
                console.log("Koneksi Websocket tidak tersedia");
            }
        }

        function toggleRelay(relayNumber, status) {
            if (socket && socket.readyState === WebSocket.OPEN) {
                socket.send(JSON.stringify({ relay: "relay_" + relayNumber, status: status }));
            } else {
                console.error("Websocket is not open. State: " + socket.readyState);
            }
        };

        document.getElementById("tombolonrelay1").addEventListener("click", function () {
            toggleRelay(1, true);
        });

        document.getElementById("tomboloffrelay1").addEventListener("click", function () {
            toggleRelay(1, false);
        });

        document.getElementById("tombolonrelay2").addEventListener("click", function () {
            toggleRelay(2, true);
        });

        document.getElementById("tomboloffrelay2").addEventListener("click", function () {
            toggleRelay(2, false);
        });

        document.getElementById("tombolonrelay3").addEventListener("click", function () {
            toggleRelay(3, true);
        });

        document.getElementById("tomboloffrelay3").addEventListener("click", function () {
            toggleRelay(3, false);
        });

        document.getElementById("tombolonrelay4").addEventListener("click", function () {
            toggleRelay(4, true);
        });

        document.getElementById("tomboloffrelay4").addEventListener("click", function () {
            toggleRelay(4, false);
        });

        document.getElementById("tombolonrelay5").addEventListener("click", function () {
            toggleRelay(5, true);
        });

        document.getElementById("tomboloffrelay5").addEventListener("click", function () {
            toggleRelay(5, false);
        });

        document.getElementById("tombolonrelay6").addEventListener("click", function () {
            toggleRelay(6, true);
        });

        document.getElementById("tomboloffrelay6").addEventListener("click", function () {
            toggleRelay(6, false);
        });



        function setTimeWaktuLampuKandangDariWebPage() {
            var xml = new XMLHttpRequest();
            var hour = document.getElementById("setJamKandang").value;
            var minute = document.getElementById("setMenitKandang").value;
            xml.onreadystatechange = function () {
                if (this.readyState == 4 && this.status == 200) {
                    document.getElementById('setLKandang').innerHTML = "Set Time: " + hour + ":" + minute;
                }
            }
            xml.open("GET", "setTimeKandang?hour" + hour + "&minute" + minute, true);
            xml.send();
        };

        function offTimeWaktuLampuKandangDariWebPage() {
            var xml = new XMLHttpRequest();
            var hour = document.getElementById("jamAkhirLKandang").value;
            var minute = document.getElementById("menitAkhirLKandang").value;
            xml.onreadystatechange = function () {
                if (this.readyState == 4 && this.status == 200) {
                    document.getElementById('setLKandang').innerHTML = "Stop Time: " + hour + ":" + minute;
                }
            }
            xml.open("GET", "stopTimeKandang?hour=" + hour + "&minute" + minute, true);
            xml.send();
        };

        setInterval(function () {
            var xml = new XMLHttpRequest();
            xml.onreadystatechange = function () {
                if (this.readyState == 4 && this.status == 200) {
                    const data = JSON.parse(this.responseText);
                    document.getElementById('_HOUR').innerHTML = data.hour;
                    document.getElementById('_MIN').innerHTML = data.minute;
                    document.getElementById('_SEC').innerHTML = data.second;
                    document.getElementById('_DAY').innerHTML = data.day;
                    document.getElementById('_MONTH').innerHTML = data.month;
                    document.getElementById('_YEAR').innerHTML = data.year;
                }
            };
            xml.open("PUT", "readWebRTC", true);
            xml.send();
        }, 1000);

        function updateInfoRelayLKandang() {
            const jamAwalLKandang = document.getElementById('setJamKandang').value;
            const menitAwalLKandang = document.getElementById('setMenitKandang').value;
            const jamAkhirLKandang = document.getElementById('offJamKandang').value;
            const menitAkhirLKandang = document.getElementById('offMenitKandang').value;
            const infoElement = document.getElementById('infoJamRelayLKandang');

            infoElement.innerHTML = `Info: Lampu akan dinyalakan pada pukul ${jamAwalLKandang}:${menitAwalLKandang} WIB dan akan dimatikan pada pukul ${jamAkhirLKandang}:${menitAkhirLKandang} WIB`;
        };

        document.getElementById('setLKandang').addEventListener('click', function () {
            updateInfoRelayLKandang();
            kirimsetTimerLampuKandang();
            kirimoffTimerLampuKandang();
        });

        function kirimsetTimerLampuKandang() {
            var houronKandang = document.getElementById("setJamKandang").value;
            var minuteonKandang = document.getElementById("setMenitKandang").value;

            var hariElements = document.querySelectorAll('#hariLKandang input[type="checkbox"]:checked');
            var hariValues = [];
            hariElements.forEach(function (el) {
                hariValues.push(el.value);
            });

            var data = {
                houronKandang: houronKandang,
                minuteonKandang: minuteonKandang,
                days: hariValues
            };

            var xml = new XMLHttpRequest();
            xml.open("POST", "/setTimeKandang", true);
            xml.setRequestHeader("Content-type", "application/json:charset=UTF-8");
            xml.onreadystatechange = function () {
                if (this.readyState == 4 && this.status == 200) {
                    alert("Timer Jam Kandang Telah diatur");
                }
            }
            xml.send(JSON.stringify(data));
        };

        function kirimoffTimerLampuKandang() {
            var houroffKandang = document.getElementById("offJamKandang").value;
            var minuteoffKandang = document.getElementById("offMenitKandang").value;

            var data = {
                houroffKandang: houroffKandang,
                minuteoffKandang: minuteoffKandang,
            };

            var xml = new XMLHttpRequest();
            xml.open("POST", "/stopTimeKandang", true);
            xml.setRequestHeader("Content-type", "application/json:charset=UTF-8");
            xml.onreadystatechange = function () {
                if (this.readyState == 4 && this.status == 200) {
                    alert("Timer off Jam Kandang Telah diatur");
                }
            };
            xml.send(JSON.stringify(data));
        };

        function setTimeWaktuLampuPemanasDariWebPage() {
            var xml = new XMLHttpRequest();
            var hour = document.getElementById('setJamPemanas').value;
            var minute = document.getElementById('setMenitPemanas').value;
            xml.onreadystatechange = function () {
                if (this.readyState == 4 && this.status == 200) {
                    document.getElementById('setLPemanas').innerHTML = "Set Time: " + hour + ":" + minute;
                }
            }
            xml.open("GET", "setTimePemanas?hour" + hour + "&minute" + minute, true);
            xml.send();
        }

        function offTimeWaktuLampuPemanasDariWebPage() {
            var xml = new XMLHttpRequest();
            var hour = document.getElementById('offJamPemanas').value;
            var minute = document.getElementById('offMenitPemanas').value;
            xml.onreadystatechange = function () {
                if (this.readyState == 4 && this.status == 200) {
                    document.getElementById('setLPemanas').innerHTML = "Set Time: " + hour + ":" + minute;
                }
            }
            xml.open("GET", "stopTimePemanas?hour=" + hour + "&minute" + minute, true);
            xml.send();
        };

        function kirimsetTimerLampuPemanas() {
            var houronPemanas = document.getElementById('setJamPemanas').value;
            var minuteonPemanas = document.getElementById('setMenitPemanas').value;

            var hariElements = document.querySelectorAll('hariLPemanas input[type="checkbox"]:checked');
            hariElements.forEach(function (el) {
                hariValues.push(el.value);
            });

            var data = {
                houronPemanas: houronPemanas,
                minuteonPemanas: minuteonPemanas,
                days: hariValues
            };

            var xml = new XMLHttpRequest();
            xml.open("POST", "/setTimePemanas", true);
            xml.setRequestHeader("Content-type", "application/json:charset=UTF-8");
            xml.onreadystatechange = function () {
                if (this.readyState == 4 && this.status == 200) {
                    alert("Timer Jam Pemanas Telah Diatur");
                }
            };

            xml.send(JSON.stringify(data));
        };

        function kirimoffTimerLampuPemanas() {
            var houroffPemanas = document.getElementById("offJamPemanas").value;
            var minuteoffPemanas = document.getElementById("offMenitPemanas").value;

            var hariElements = document.querySelectorAll('#hariLPemanas input[type="checkbox"]:checked');
            var hariValues = [];
            hariElements.forEach(function (el) {
                hariValues.push(el.value);
            });

            var data = {
                houroffPemanas: houroffPemanas,
                minuteoffPemanas: minuteoffPemanas,
            };

            var xml = new XMLHttpRequest();
            xml.open("POST", "/stopTimePemanas", true);
            xml.setRequestHeader("Content-type", "application/json:charset=UTF-8");
            xml.onreadystatechange = function () {
                if (this.readyState == 4 && this.status == 200) {
                    alert("Timer off Jam Pemanas telah diatur");
                }
            };
            xml.send(JSON.stringify(data));
        };

        function updateInfoRelayLPemanas() {
            const jamAwalLPemanas = document.getElementById('setJamPemanas').value;
            const menitAwalLPemanas = document.getElementById('setMenitPemanas').value;
            const jamAkhirLPemanas = document.getElementById('offJamPemanas').value;
            const menitAkhirLPemanas = document.getElementById('offMenitPemanas').value;
            const infoElementPemanas = document.getElementById('infoJamRelayLPemanas');

            infoElementPemanas.innerHTML = `Info: Lampu akan dinyalakan pada pukul ${jamAwalLPemanas}:${menitAwalLPemanas} WIB dan akan dimatikan pada pukul ${jamAkhirLPemanas}:${menitAkhirLPemanas} WIB`;
        };

        document.getElementById('setLPemanas').addEventListener('click', function () {
            updateInfoRelayLPemanas();
            kirimsetTimerLampuPemanas();
            kirimoffTimerLampuPemanas();
        });

        function kirimStartPeriodeKeMikrokontroler(formattedStartDate, formattedEndDate) {
            //kirim variabel startDate ke mikrokontroler melalui websocket
            if (socket && socket.readyState === WebSocket.OPEN) {
                socket.send(JSON.stringify({ startdate: startDate }));
            } else {
                console.log("Tanggal Start Gagal Dikirim");
                console.log("Koneksi Websocket tidak tersedia");
            }
        }


        document.getElementById("kirimStartTanggal").addEventListener('click', function () {
            kirimStartPeriodeKeMikrokontroler();
        });


        // Function to display status messages
        function displayStatusMessage(message, isError = false) {
            const statusMessage = document.getElementById("statusMessage");
            statusMessage.textContent = message;
            statusMessage.style.color = isError ? 'red' : 'green';
        }


        document.addEventListener("DOMContentLoaded", function () {
            const form = document.getElementById("formRegistrasi");
            const startDateInput = document.getElementById("startDate");
            const endDateInput = document.getElementById("endDate");
            const formattedStartDateInput = document.getElementById("formattedStartDate");
            const formattedEndDateInput = document.getElementById("formattedEndDate");

            // Helper function to add days to a date
            function tambahHariOtomatis(date, days) {
                let result = new Date(date);
                result.setDate(result.getDate() + days);
                return result;
            }

            // Helper function to format date as DD-MM-YYYY
            function formatDateInput(date) {
                let day = date.getDate().toString().padStart(2, '0');
                let month = (date.getMonth() + 1).toString().padStart(2, '0'); // Months are zero-based
                let year = date.getFullYear();
                return `${day}-${month}-${year}`;
            }


            // Event listener for startDateInput change
            startDateInput.addEventListener("change", function () {
                if (startDateInput.value) {
                    let selectedDate = new Date(startDateInput.value);
                    // Display formatted start date
                    formattedStartDateInput.value = formatDateInput(selectedDate);

                    let endDate = tambahHariOtomatis(selectedDate, 34);
                    endDateInput.value = formatDateInput(endDate);
                    formattedEndDateInput.value = formatDateInput(endDate);
                } else {
                    // Clear the inputs if no date is selected
                    endDateInput.value = "";
                    formattedStartDateInput.value = "";
                    formattedEndDateInput.value = "";
                }
            });

            // Event listener for form submission
            form.addEventListener("submit", function (event) {
                event.preventDefault(); // Prevent default form submission behavior

                if (startDateInput.value) {
                    let selectedDate = new Date(startDateInput.value);
                    let endDate = new Date(endDateInput.value);
                    let formattedStartDate = formatDateInput(selectedDate);
                    let formattedEndDate = formatDateInput(endDate);
                    kirimStartPeriodeKeMikrokontroler(formattedStartDate);
                } else {
                    displayStatusMessage("Silakan pilih Tanggal Start terlebih dahulu.", true);
                }
            });
        });




        window.onload = init;




    </script>
</body>

</html>
)=====";
