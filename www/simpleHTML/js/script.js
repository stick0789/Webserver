const burger = document.getElementById('burgerMenu');
const sidebar = document.getElementById('sidebar');
const burgerIcon = burger.querySelector('i');

burger.onclick = function() {
    sidebar.classList.toggle('open');
    burger.classList.toggle('open');
    // Cambia el icono entre burger y X
    if (sidebar.classList.contains('open')) {
        burgerIcon.classList.remove('fa-bars');
        burgerIcon.classList.add('fa-xmark');
    } else {
        burgerIcon.classList.remove('fa-xmark');
        burgerIcon.classList.add('fa-bars');
    }
};