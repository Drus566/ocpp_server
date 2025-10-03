document.addEventListener('DOMContentLoaded', function () {
	const tooltipIcons = document.querySelectorAll('.tooltip-icon');

	tooltipIcons.forEach(icon => {
		const tooltipText = icon.getAttribute('data-tooltip');
		const tooltip = document.createElement('div');
		tooltip.className = 'tooltip';
		tooltip.textContent = tooltipText;
		icon.appendChild(tooltip);

		icon.addEventListener('mouseenter', function () {
			tooltip.classList.add('tooltip--visible');
		});

		icon.addEventListener('mouseleave', function () {
			tooltip.classList.remove('tooltip--visible');
		});
	});
});