<script lang="ts">
	import { base } from '$app/paths';
	let current_year: number = new Date().getFullYear();
</script>

<head>
	<link
		href="https://fonts.googleapis.com/css2?family=Fira+Code:wght@300..700&family=Noto+Serif+JP:wght@200..900&family=Spectral:ital,wght@0,200;0,300;0,400;0,500;0,600;0,700;0,800;1,200;1,300;1,400;1,500;1,600;1,700;1,800&display=swap"
		rel="stylesheet"
	/>
</head>

<div id="layout-container">
	<header>
		<nav>
			<ul>
				<li><a href="{base}/" id="migopp">migopp🔥</a></li>
				<li><a href="{base}/thoughts">thoughts</a></li>
				<li><a href="{base}/projects">projects</a></li>
			</ul>
		</nav>
	</header>

	<main>
		<slot></slot>
	</main>

	<footer>
		<ul>
			<li class="jp">僕の心から</li>
			<li><a href="https://github.com/migopp">github</a></li>
			<li><a href="https://linkedin.com/in/migopp">linkedin</a></li>
		</ul>
		<p>&copy; {current_year} michael goppert</p>
	</footer>
</div>

<style>
	#layout-container {
		display: flex;
		flex-direction: column;
		max-width: 900px;
		margin: 0 auto;
		font-family: 'Spectral', monospace;
	}

	#migopp {
		font-size: 1.5rem;
		font-weight: 600;
	}

	.jp {
		font-family: 'Noto Serif JP', serif;
		font-weight: 400;
	}

	* {
		margin: 0;
		padding: 0;
	}

	header,
	main {
		height: 100%;
	}

	main {
		flex: 1;
	}

	header,
	footer {
		margin: 1rem 0;
	}

	header ul,
	footer ul {
		list-style: none;
		display: flex;
		flex-direction: row;
		align-items: center;
	}
	header li,
	footer li {
		font-size: 1.1rem;
		margin-right: 1rem;
	}

	header li a,
	footer li a {
		text-decoration: none;
		color: inherit;
	}
</style>
