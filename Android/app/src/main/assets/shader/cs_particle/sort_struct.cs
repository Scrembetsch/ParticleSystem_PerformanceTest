#version VERSION

#define eLocalBms      0U
#define eLocalDisperse 1U
#define eBigFlip       2U
#define eBigDisperse   3U

precision mediump float;

layout(local_size_x = LOCAL_SIZE_X) in;

struct Particle
{
    vec4 Position;
    vec4 Velocity;
    vec4 Color;
    vec4 Lifetime;
};

// Buffer always uses vec4
layout(std140, binding=1) buffer ParticleBuffer
{
    Particle Particles[];
};

uniform uint uAlgorithm;
uniform uint uN;
uniform uint uH;

// Workgroup local memory. We use this to minimise round-trips to global memory.
// It allows us to evaluate a sorting network of up to 1024 with one shader invocation.
shared Particle local_particles[LOCAL_SIZE_X * 2];

void Swap(inout Particle x, inout Particle y)
{
	Particle tmp = x;
	x = y;
	y = tmp;
}

bool IsSmaller(float x, float y)
{
	return x < y;
}

void global_compare_and_swap(uvec2 idx){
	if (IsSmaller(Particles[idx.x].Position.w, Particles[idx.y].Position.w))
    {
        Swap(Particles[idx.x], Particles[idx.y]);
	}
}

void big_flip( in uint n, in uint h) {

	//	uint n                  // total number of sortable elements
	//	uint h                  // flip height
	//	uint gl_WorkGroupSize.x // number of threads in block/workgroup: 
                                // each thread deals with two sortable elements

	if (gl_WorkGroupSize.x * 2U > h) {
		return;
	}

	uint t_prime = gl_GlobalInvocationID.x;
	uint half_h = h >> 1; // Note: h >> 1 is equivalent to h / 2 

	uint q       = ((2U * t_prime) / h) * h;
	uint x       = q     + (t_prime % half_h);
	uint y       = q + h - (t_prime % half_h) - 1U; 


	global_compare_and_swap(uvec2(x,y));
}


void big_disperse( in uint n, in uint h ) {

	//	uint n                  // total number of sortable elements
	//	uint h                  // disperse height
	//	uint gl_WorkGroupSize.x // number of threads in block/workgroup: 
                                // each thread deals with two sortable elements

	if ( gl_WorkGroupSize.x * 2U > h ) {
		return;
	};

	uint t_prime = gl_GlobalInvocationID.x;

	uint half_h = h >> 1; // Note: h >> 1 is equivalent to h / 2 

	uint q       = ((2U * t_prime) / h) * h;
	uint x       = q + (t_prime % (half_h));
	uint y       = q + (t_prime % (half_h)) + half_h;

	global_compare_and_swap(uvec2(x,y));

}

// Performs compare-and-swap over elements held in shared,
// workgroup-local memory
void local_compare_and_swap(uvec2 idx){
	if (IsSmaller(local_particles[idx.x].Position.w, local_particles[idx.y].Position.w))
    {
        Swap(local_particles[idx.x], local_particles[idx.y]);
	}
}

// Performs full-height flip (h height) over locally available indices.
void local_flip(in uint h){
		uint t = gl_LocalInvocationID.x;
		barrier();

		uint half_h = h >> 1; // Note: h >> 1 is equivalent to h / 2 
		uvec2 indices = 
			uvec2( h * ( ( 2U * t ) / h ) ) +
			uvec2( t % half_h, h - 1U - ( t % half_h ) );

		local_compare_and_swap(indices);
}

// Performs progressively diminishing disperse operations (starting with height h)
// on locally available indices: e.g. h==8 -> 8 : 4 : 2.
// One disperse operation for every time we can half h.
void local_disperse(in uint h){
	uint t = gl_LocalInvocationID.x;
	for ( ; h > 1U ; h /= 2U ) {
		
		barrier();

		uint half_h = h >> 1; // Note: h >> 1 is equivalent to h / 2 
		uvec2 indices = 
			uvec2( h * ( ( 2U * t ) / h ) ) +
			uvec2( t % half_h, half_h + ( t % half_h ) );

		local_compare_and_swap(indices);
	}
}

// Perform binary merge sort for local elements, up to a maximum number 
// of elements h.
void local_bms(uint h){
	uint t = gl_LocalInvocationID.x;
	for ( uint hh = 2U; hh <= h; hh <<= 1U ) {  // note:  h <<= 1 is same as h *= 2
		local_flip( hh);
		local_disperse( hh/2U);
	}
}

void main(){

	// this shader can be called in four different modes:
	// 1. local flip+disperse (up to n == local_size_x * 2) 
	// 2. big flip
	// 3. big disperse
	// 4. local disperse 
	// the total number of elements 

	uint t = gl_LocalInvocationID.x;

    // Calculate global offset for local workgroup
    //
	uint offset = gl_WorkGroupSize.x * 2U * gl_WorkGroupID.x; 

	if (uAlgorithm <= eLocalDisperse){
		// pull to local memory
	    // Each local worker must save two elements to local memory, as there
	    // are twice as many elments as workers.
		local_particles[t*2U]   = Particles[offset+t*2U];
		local_particles[t*2U+1U] = Particles[offset+t*2U+1U];
	}

	uint n = uN;

	switch (uAlgorithm){
		case eLocalBms:
			local_bms(uH);
		break;
		case eLocalDisperse:
			local_disperse(uH);
		break;
		case eBigFlip:
			big_flip(uN, uH);
		break;
		case eBigDisperse:
			big_disperse(uN, uH);
		break;
	}

	// Write local memory back to buffer

	if (uAlgorithm <= eLocalDisperse){
		barrier();
		// push to global memory
		Particles[offset+t*2U]   = local_particles[t*2U];
		Particles[offset+t*2U+1U] = local_particles[t*2U+1U];
	}
}